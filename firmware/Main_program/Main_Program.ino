#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <BLEDevice.h>


static const char *WIFI_SSID = "-";
static const char *WIFI_PASSWORD = "-";

static const char *LOTUS_MAC = "be:16:15:01:d4:ed";
static BLEUUID lotusService((uint16_t)0xFFF0);
static BLEUUID lotusCharacteristic((uint16_t)0xFFF3);

WebServer server(80);
BLEClient *lotusClient = nullptr;
BLERemoteCharacteristic *lotusWriter = nullptr;

portMUX_TYPE queueMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t pendingColor = 0;
volatile bool hasPending = false;
uint32_t lastColorSent = 0xFFFFFFFF;
uint32_t lastRgb = 0x00FF8800;
uint32_t lastConnectTry = 0;

static const char COLOR_PAGE[] PROGMEM = R"HTML(
<!doctype html><html><head><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Lotus Lights</title><style>
body{font:18px system-ui;text-align:center;background:#111;color:#fff;margin:2em auto;max-width:420px}
button,input{font:inherit;margin:12px;padding:14px;border:0;border-radius:12px}
button{width:42%;background:#333;color:white}input[type=color]{width:90%;height:90px;background:#222}
#status{color:#aaa}
</style></head><body><h1>Lotus Lights</h1>
<button onclick="send('/on')">On</button><button onclick="send('/off')">Off</button>
<p><input id="c" type="color" value="#ff8800" onchange="color()"></p><p id="status">Ready</p>
<script>
async function send(path){try{let r=await fetch(path);document.querySelector('#status').textContent=await r.text()}catch(e){document.querySelector('#status').textContent='Connection failed'}}
function color(){let h=document.querySelector('#c').value.slice(1);send('/color?hex='+h)}
</script></body></html>
)HTML";

void queueRgb(bool on, uint8_t r, uint8_t g, uint8_t b) {
  uint32_t value = (on ? 0x01000000UL : 0) | ((uint32_t)r << 16) |
                   ((uint32_t)g << 8) | b;
  portENTER_CRITICAL(&queueMux);
  pendingColor = value;
  hasPending = true;
  portEXIT_CRITICAL(&queueMux);
}

void queuePowerOn() {
  uint32_t value = 0x03000000UL | lastRgb; // On + force the controller's ON frame.
  portENTER_CRITICAL(&queueMux);
  pendingColor = value;
  hasPending = true;
  portEXIT_CRITICAL(&queueMux);
}

bool connectLotus() {
  if (lotusClient && lotusClient->isConnected() && lotusWriter) return true;
  // Avoid repeatedly blocking the web server while a reconnect is in progress.
  if (millis() - lastConnectTry < 4000) return false;
  lastConnectTry = millis();
  lotusWriter = nullptr;
  if (!lotusClient) lotusClient = BLEDevice::createClient();
  if (!lotusClient || !lotusClient->connect(BLEAddress(LOTUS_MAC), 0x00, 3000)) return false;

  BLERemoteService *service = lotusClient->getService(lotusService);
  if (!service) { lotusClient->disconnect(); return false; }
  lotusWriter = service->getCharacteristic(lotusCharacteristic);
  if (!lotusWriter || (!lotusWriter->canWrite() && !lotusWriter->canWriteNoResponse())) {
    lotusWriter = nullptr;
    lotusClient->disconnect();
    return false;
  }
  Serial.println("Lotus BLE connected");
  return true;
}

bool writeLotus(const uint8_t *data, size_t size, bool waitForAck = false) {
  if (!connectLotus()) return false;
  // Prefer Write Without Response when the Lotus characteristic supports it.
  bool response = waitForAck || !lotusWriter->canWriteNoResponse();
  return lotusWriter->writeValue((uint8_t *)data, size, response);
}

void processLight() {
  uint32_t value;
  portENTER_CRITICAL(&queueMux);
  if (!hasPending) { portEXIT_CRITICAL(&queueMux); return; }
  value = pendingColor;
  hasPending = false;
  portEXIT_CRITICAL(&queueMux);
  bool forceOn = value & 0x02000000UL;
  if (!forceOn && value == lastColorSent) return;

  bool on = value & 0x01000000UL;
  uint8_t r = value >> 16, g = value >> 8, b = value;
  static const uint8_t offCommand[] = {0x7E,0x04,0x04,0x00,0x00,0x00,0xFF,0x00,0xEF};
  static const uint8_t onCommand[]  = {0x7E,0x04,0x04,0xF0,0x00,0x01,0xFF,0x00,0xEF};
  uint8_t colorCommand[] = {0x7E,0x07,0x05,0x03,r,g,b,0x10,0xEF};

  bool sameColor = lastColorSent != 0xFFFFFFFF &&
                   (lastColorSent & 0x00FFFFFFUL) == (value & 0x00FFFFFFUL);
  bool ok = forceOn ? writeLotus(onCommand, sizeof(onCommand), true) :
            !on ? writeLotus(offCommand, sizeof(offCommand)) :
            (!(lastColorSent & 0x01000000UL) && sameColor) ?
              writeLotus(onCommand, sizeof(onCommand)) :
              writeLotus(colorCommand, sizeof(colorCommand));
  if (ok) {
    lastColorSent = value & ~0x02000000UL;
    if (on) lastRgb = value & 0x00FFFFFFUL;
  }
  else {
    portENTER_CRITICAL(&queueMux);
    pendingColor = value;
    hasPending = true;
    portEXIT_CRITICAL(&queueMux);
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setHostname("lotus-bridge");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print('.'); }
  Serial.printf("\nWi-Fi connected: http://%s/\n", WiFi.localIP().toString().c_str());

  // Free Classic Bluetooth RAM; this firmware does not use Matter or Classic BT.
  esp_bt_mem_release(ESP_BT_MODE_CLASSIC_BT);
  if (!BLEDevice::init("")) Serial.println("BLE init failed");

  server.on("/", HTTP_GET, []() { server.send_P(200, "text/html", COLOR_PAGE); });
  server.on("/on", HTTP_GET, []() {
    queuePowerOn(); server.send(200, "text/plain", "ON queued");
  });
  server.on("/off", HTTP_GET, []() { queueRgb(false, 0, 0, 0); server.send(200, "text/plain", "OFF queued"); });
  server.on("/color", HTTP_GET, []() {
    uint8_t r, g, b;
    if (server.hasArg("hex") && server.arg("hex").length() == 6) {
      String h = server.arg("hex");
      char *end;
      r = strtoul(h.substring(0, 2).c_str(), &end, 16);
      g = strtoul(h.substring(2, 4).c_str(), &end, 16);
      b = strtoul(h.substring(4, 6).c_str(), &end, 16);
    } else if (server.hasArg("r") && server.hasArg("g") && server.hasArg("b")) {
      int ri = server.arg("r").toInt(), gi = server.arg("g").toInt(), bi = server.arg("b").toInt();
      if (ri < 0 || ri > 255 || gi < 0 || gi > 255 || bi < 0 || bi > 255) {
        server.send(400, "text/plain", "RGB values must be 0-255"); return;
      }
      r = ri; g = gi; b = bi;
    } else { server.send(400, "text/plain", "Use hex=RRGGBB or r,g,b"); return; }
    queueRgb(true, r, g, b); server.send(200, "text/plain", "Color queued");
  });
  server.begin();
}

void loop() {
  server.handleClient();
  processLight();
  if ((!lotusClient || !lotusClient->isConnected()) && millis() - lastConnectTry >= 4000) {
    connectLotus();
  }
  delay(5);
}
