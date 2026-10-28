#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
BLEScan* scanner;
class ScanCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice device) override {
    Serial.print("Found: ");
    Serial.print(device.getAddress().toString().c_str());

    Serial.print(" | RSSI: ");
    Serial.print(device.getRSSI());

    if (device.haveName()) {
      Serial.print(" | Name: ");
      Serial.print(device.getName().c_str());
    }

    Serial.println();
  }
};
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- BLE scanner starting ---");
  BLEDevice::init("");
  scanner = BLEDevice::getScan();
  scanner->setAdvertisedDeviceCallbacks(
    new ScanCallbacks()
  );
  scanner->setActiveScan(true);
  scanner->setInterval(100);
  scanner->setWindow(99);
}
void loop() {
  Serial.println("\nScanning for 5 seconds...");
  scanner->start(5, false);
  scanner->clearResults();
  delay(2000);
}
