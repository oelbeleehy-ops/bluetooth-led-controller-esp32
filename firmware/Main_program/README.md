# Main firmware

`Main_program.ino` is the V1 Wi-Fi and BLE bridge. Replace the Wi-Fi placeholders in the sketch with your own 2.4 GHz network name and password before uploading it to the ESP32. Never commit your actual credentials.

This build uses the classic ESP32 Bluetooth stack as a BLE GATT client and a local HTTP server. It is not a Matter accessory. See the root README and `docs/DEVELOPMENT_NOTES.md` for the tested limits and setup notes.
