# Lotus Lantern ESP32 Bridge — V1

An ESP32-WROOM-32 bridge for controlling a Lotus Lantern LED controller from an iPhone. V1 uses a local Wi-Fi web page and Apple Shortcuts; the ESP32 sends the known Lotus commands over BLE.

## V1 status

- The ESP32 has connected to the Lotus Lantern controller over BLE.
- The controller was identified as `be:16:15:01:d4:ed`, service `FFF0`, characteristic `FFF3`.
- The documented ON, OFF, and RGB command frames are recorded in [the development notes](docs/DEVELOPMENT_NOTES.md).
- Matter was tested separately, but Matter plus the BLE client exceeded available runtime memory on the tested ESP32-WROOM-32 (no PSRAM). The Shortcut bridge is the practical V1 direction for this board.
- The current firmware uses HTTP on the home LAN. Remote access is not configured.

## Repository contents

```text
firmware/Main_program/Main_program.ino
tools/Bluetooth_device_scanner/       scanner source pending
docs/DEVELOPMENT_NOTES.md             Hardware, discovery, experiments, and known limits
docs/SHORTCUTS.md                     iPhone Shortcut and automation setup
```

The Main program sketch is included. The original `Bluetooth_device_scanner.ino` source is not present in the files available for this repository yet, so the scanner folder currently contains its documentation only. Add the original sketch when it is available; do not substitute a guessed recreation for the discovery tool the user actually ran.

## Hardware and software

- Board: ESP32-WROOM-32 development board
- Controller: Lotus Lantern BLE LED controller
- Arduino IDE with Espressif Arduino-ESP32 core 3.3.12 was used during V1 troubleshooting
- The WROOM board tested reports no PSRAM

## Wi-Fi credentials

In the sketch, `WIFI_SSID` means your Wi-Fi network name and `WIFI_PASSWORD` means the password you use to join that network. Enter both on your own computer only. Do not commit them. If your source uses a `secrets.h`, it is already excluded by `.gitignore`; commit only an example file with blank placeholders. A real Wi-Fi password was shared during development, so change it before publishing if it is still active.

## Setup overview

1. Upload `Main_program.ino` to the ESP32.
2. Keep the ESP32 and iPhone on the same home Wi-Fi network.
3. Read the ESP32's current IP address from Serial Monitor.
4. In Shortcuts, use **URL** followed by **Get Contents of URL** with the bridge endpoint. See [Shortcuts setup](docs/SHORTCUTS.md).

## License

No license has been selected yet. Add a license before inviting reuse or accepting contributions.

## Publish safely

The Wi-Fi name and password are not included in this repository. Set them in the sketch on your own computer before flashing it. The password was shared during development; change it before publishing if it is still active. The scanner source should be added when available. Do not upload Arduino build files or private network details.

## Planned V2

Revisit native Apple Home control after choosing a supported home hub and testing the exact model/software combination. A hub does not turn the current HTTP Shortcut endpoint into a Home accessory. See the development notes before buying hardware.
