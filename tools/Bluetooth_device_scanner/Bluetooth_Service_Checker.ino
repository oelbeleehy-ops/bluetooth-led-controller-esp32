#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEClient.h>
#include <BLERemoteService.h>
#include <BLERemoteCharacteristic.h>
static BLEAddress targetAddress("be:16:15:01:d4:ed");
static bool done = false;
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting BLE discovery...");
  BLEDevice::init("");
  BLEClient* client = BLEDevice::createClient();
  Serial.println("Connecting to MELK-OA21 ED...");
  if (!client->connect(targetAddress)) {
    Serial.println("Connection failed.");
    return;
  }
  Serial.println("Connected!");
  std::map<std::string, BLERemoteService*>* services =
      client->getServices();
  for (auto const& entry : *services) {
    BLERemoteService* service = entry.second;
    Serial.print("\nSERVICE: ");
    Serial.println(service->getUUID().toString().c_str());
    std::map<std::string, BLERemoteCharacteristic*>* chars =
        service->getCharacteristics();
    for (auto const& item : *chars) {
      BLERemoteCharacteristic* ch = item.second;
      Serial.print("  CHARACTERISTIC: ");
      Serial.print(ch->getUUID().toString().c_str());
      Serial.print(" | Read: ");
      Serial.print(ch->canRead());
      Serial.print(" | Write: ");
      Serial.print(ch->canWrite());
      Serial.print(" | Write NR: ");
      Serial.print(ch->canWriteNoResponse());
      Serial.print(" | Notify: ");
      Serial.println(ch->canNotify());
    }
  }
  Serial.println("\nDiscovery complete.");
  done = true;
}
void loop() {
  delay(1000);
}
