# Bluetooth LED Light To ESP32 Bridge — V1

An ESP32-WROOM-32 bridge for controlling an LED controller from an iPhone to implement automations. V1 uses a local Wi-Fi web page and Apple Shortcuts; the ESP32 sends the known commands over BLE.

## V1 status

- The ESP32 has connected to the LED controller over BLE.
- The controller was identified as `be:16:15:01:d4:ed`, service `FFF0`, characteristic `FFF3`.
- Matter was tested separately, but Matter plus the BLE client exceeded available runtime memory on the tested ESP32-WROOM-32 (no PSRAM). The Shortcut bridge is the practical V1 direction for this board.
- The current firmware uses HTTP on the home LAN. Remote access is not configured.

## Repository contents

```text
firmware/Main_program/Main_program.ino
tools/Bluetooth_device_scanner/       scanner source pending
docs/DEVELOPMENT_NOTES.md             Hardware, discovery, experiments, and known limits
docs/SHORTCUTS.md                     iPhone Shortcut and automation setup
```

## Hardware and software

- Board: ESP32-WROOM-32 development board
- Controller: BLE LED controller
- Arduino IDE with Espressif Arduino-ESP32 core 3.3.12 was used during V1 troubleshooting
- The WROOM board tested reports no PSRAM

## Automations and runtime test

The ESP32 bridge ran continuously for 72 hours while two iPhone Shortcut automations were active:

- Turn the lights on at sunset and off at sunrise.
- Change the light colour based on the time of day.

The schedules were triggered by Shortcuts; they are not stored on the ESP32.

-upon startup, the ESP32 outputs an IP-address that can be used to access the HTML page where you can interact with the lights without use of the shortcuts.

## Wi-Fi credentials

In the sketch, `WIFI_SSID` means your Wi-Fi network name and `WIFI_PASSWORD` means the password you use to join that network. They have been omitted for privacy reasons.

## Setup overview

1. Upload `Main_program.ino` to the ESP32.
2. Keep the ESP32 and iPhone on the same home Wi-Fi network.
3. Read the ESP32's current IP address from Serial Monitor.
4. In Shortcuts, use **IP-Address** followed by **Get Contents of URL** with the bridge endpoint. See [Shortcuts setup](docs/SHORTCUTS.md).

## Planned V2

- Revisit native Apple Home control after choosing a supported home hub and testing the exact model/software combination. A hub does not turn the current HTTP Shortcut endpoint into a Home accessory. See the development notes before buying hardware.
- Potentially implement a local AI model running on a home server for smarter assistance and more complex automations
