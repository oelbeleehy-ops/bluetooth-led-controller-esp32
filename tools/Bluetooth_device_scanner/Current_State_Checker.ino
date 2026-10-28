#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEClient.h>
#include <BLERemoteService.h>
#include <BLERemoteCharacteristic.h>
static BLEAddress targetAddress("be:16:15:01:d4:ed");
void setup() {
  Serial.begin(115200);
  delay(1000);
  BLEDevice::init("");
  BLEClient* client = BLEDevice::createClient();
  Serial.println("Connecting...");
  if (!client->connect(targetAddress)) {
    Serial.println("Connection failed.");
    return;
  }
  Serial.println("Connected!");
  BLERemoteService* service =
      client->getService(BLEUUID((uint16_t)0xFFF0));
  if (service == nullptr) {
    Serial.println("FFF0 service not found.");
    return;
  }
  BLERemoteCharacteristic* ch =
      service->getCharacteristic(BLEUUID((uint16_t)0xFFF3));
  if (ch == nullptr) {
    Serial.println("FFF3 characteristic not found.");
    return;
  }
  if (ch->canRead()) {
    String value = ch->readValue();
    Serial.print("FFF3 data (hex): ");
    for (int i = 0; i < value.length(); i++) {
      uint8_t b = (uint8_t)value[i];
      if (b < 16) Serial.print("0");
      Serial.print(b, HEX);
      Serial.print(" ");
}

Serial.println();
  } else {
    Serial.println("FFF3 is not readable.");
  }
  client->disconnect();
  Serial.println("Read test complete.");
}
void loop() {
  delay(1000);
}
