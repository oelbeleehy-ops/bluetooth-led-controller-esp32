#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEClient.h>
#include <BLERemoteService.h>
#include <BLERemoteCharacteristic.h>
static BLEAddress targetAddress("be:16:15:01:d4:ed");
BLEClient* client = nullptr;
BLERemoteCharacteristic* led = nullptr;
bool connectLED() {
  if (client && client->isConnected()) return true;
  if (!client) client = BLEDevice::createClient();
  Serial.println("Connecting to LED controller...");
  if (!client->connect(targetAddress)) {
    Serial.println("Connection failed.");
    return false;
  }
  BLERemoteService* service =
      client->getService(BLEUUID((uint16_t)0xFFF0));
  if (!service) {
    Serial.println("FFF0 service not found.");
    client->disconnect();
    return false;
  }
  led = service->getCharacteristic(BLEUUID((uint16_t)0xFFF3));
  if (!led || !led->canWriteNoResponse()) {
    Serial.println("Writable FFF3 characteristic not found.");
    client->disconnect();
    return false;
  }
  Serial.println("LED controller connected!");
  return true;
}
void sendCommand(uint8_t* data, size_t len) {
  if (!connectLED()) return;

  bool ok = led->writeValue(data, len, false);
  Serial.println(ok ? "Command sent." : "Write failed.");
}
void powerLED(bool on) {
  uint8_t cmdOn[]  = {0x7E, 0x04, 0x04, 0xF0, 0x00, 0x01, 0xFF, 0x00, 0xEF};
  uint8_t cmdOff[] = {0x7E, 0x04, 0x04, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xEF};
  sendCommand(on ? cmdOn : cmdOff, 9);
}

void setColour(uint8_t r, uint8_t g, uint8_t b) {
  uint8_t cmd[] = {0x7E, 0x07, 0x05, 0x03, r, g, b, 0x10, 0xEF};
  sendCommand(cmd, 9);
}
void printMenu() {
  Serial.println("\n--- LED CONTROL ---");
  Serial.println("1 = ON");
  Serial.println("0 = OFF");
  Serial.println("R = Red");
  Serial.println("G = Green");
  Serial.println("B = Blue");
  Serial.println("W = White");
  Serial.println("Type a command and press Enter.");
}
void setup() {
  Serial.begin(115200);
  delay(1000);
  BLEDevice::init("");
  printMenu();
}
void loop() {
  if (!Serial.available()) {
    delay(20);
    return;
  }
  char c = Serial.read();
  switch (c) {
    case '1': powerLED(true); break;
    case '0': powerLED(false); break;
    case 'R': case 'r': setColour(255, 0, 0); break;
    case 'G': case 'g': setColour(0, 255, 0); break;
    case 'B': case 'b': setColour(0, 0, 255); break;
    case 'W': case 'w': setColour(255, 255, 255); break;
    case '\n': case '\r': break;
    default: Serial.println("Unknown command."); break;
  }
}
