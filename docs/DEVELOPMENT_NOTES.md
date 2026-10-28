# V1 development notes

## Goal

Control an LED strip/controller from Apple devices, using an ESP32-WROOM-32.

## BLE discovery and confirmed controller details

The scanner sketch was used to find and inspect the LED BLE controller.
After searching, the LED strip was identified using the scanner as: 
Device found: Name: MELK-OA21   ED, Address: be:16:15:01:d4:ed, rssi: -67

After further testing, I found:
- Service UUID: `FFF0`
- Writable characteristic UUID: `FFF3`

## Matter experiment

The first size error showed a 2,310,371-byte sketch against a selected 1,310,720-byte app partition. Selecting **Huge APP (3 MB No OTA / 1 MB SPIFFS)** was required for the Matter build.

Runtime tests:

- Matter-only test stayed running on the WROOM-32. Serial output showed 167,848 bytes of heap before the Matter endpoint, 134,856 bytes after Matter startup, and 0 bytes of PSRAM.
- Starting BLE before the Matter endpoint then caused a crash while `MatterColorLight::begin()` created the endpoint.
- Starting Matter before BLE instead caused a Bluedroid `fixed_queue_new` assertion during BLE initialization.

These tests indicate the tested WROOM-32 does not have enough usable runtime memory for this Matter color endpoint and the BLE client at the same time. A board with more memory may be worth testing, but it is not a verified fix for the combined Matter-plus-BLE use case. The working V1 direction is Wi-Fi + BLE without Matter.

The reboot evidence was two distinct failures:

1. Matter first, followed by BLE initialization: `fixed_queue_new` asserted in the Bluetooth stack.
2. BLE first, followed by Matter endpoint creation: the program aborted while creating the Matter endpoint.

The later Matter-only boot reached “Matter started” and printed a pairing code without rebooting. This isolated the conflict to combining the components, rather than Wi-Fi alone.

## Shortcuts bridge direction

The bridge serves a small local web page with a color picker and on/off buttons, and supports HTTP GET endpoints for Shortcuts:

- `/on`
- `/off`
- `/color?hex=RRGGBB`
- `/color?r=R&g=G&b=B`

The bridge prints its current IP address to Serial Monitor. Use the current printed address in shortcuts.


### Future V2: native Apple Home

- The Shortcuts bridge is not a Home app accessory. A hub enables remote control of Apple Home accessories; remote Home control requires a home hub. (Hopefully used in V2 or V3)

