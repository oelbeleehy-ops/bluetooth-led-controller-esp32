# V1 development notes

## Goal

Control a Lotus Lantern LED strip/controller from Apple devices, using an ESP32-WROOM-32. The user has an iPhone running iOS 27 and initially wanted a color wheel in Apple Home without a HomePod or Apple TV.

## BLE discovery and confirmed controller details

The scanner sketch was used to find and inspect the Lotus Lantern BLE controller. The user reports a working connection to:

- BLE address: `be:16:15:01:d4:ed`
- Service UUID: `FFF0`
- Writable characteristic UUID: `FFF3`

Known command frames (hex bytes):

| Operation | Frame |
|---|---|
| ON | `7E 04 04 F0 00 01 FF 00 EF` |
| OFF | `7E 04 04 00 00 00 FF 00 EF` |
| RGB colour | `7E 07 05 03 R G B 10 EF` |

`R`, `G`, and `B` are byte values from `00` to `FF`. The user's scanner source has not yet been added to this repository; add it under `tools/Bluetooth_device_scanner/`.

## Matter experiment

The Arduino Matter library can make a Matter color-light endpoint, but the original ESP32 target uses on-network Wi-Fi commissioning. A compile failure caused by naming a BLE pointer `client`, which conflicts with `esp_matter::client`; renaming the pointer to `lotusClient` addressed that name collision.

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

The bridge prints its current IP address to Serial Monitor. The example IP seen during testing was `192.168.1.199`; this may change unless the router reserves that address. Use the current printed address in shortcuts.

V1 responsiveness changes prefer BLE Write Without Response when supported, use an acknowledged write for the explicit ON command, shorten reconnect timeouts, and disable Wi-Fi sleep. The user reported that all functions except ON worked before the explicit ON-frame adjustment; that adjustment still needs confirmation on-device.

## Remote access and Apple Home notes

The Shortcuts bridge uses a private LAN IP and plain HTTP. Those URLs only work while the iPhone can reach the home LAN, for example via a VPN into the home network. Do not expose these unauthenticated HTTP endpoints with router port forwarding.

The Shortcuts bridge is not a Home app accessory. An Apple home hub does not automatically proxy these custom HTTP Shortcut requests. A hub enables remote control of Apple Home accessories; remote Home control requires a home hub ([Apple support](https://support.apple.com/en-gb/guide/iphone/iph1d10f7f2b/ios)). Check the exact model and software support before buying a used hub; “any generation” is not a safe assumption. Some Matter Wi-Fi accessories can be added and controlled locally from supported iPhones without a hub, but that does not solve this bridge's WROOM memory constraint ([Apple Matter support](https://support.apple.com/en-gb/102135)).

### Future V2: native Apple Home

When revisiting Matter, first select the target ESP32 hardware and confirm memory headroom for Wi-Fi, Matter, and the Lotus BLE client together. Test boot and commissioning on that exact hardware before treating it as a replacement for V1. Separately confirm the chosen Apple home hub model and software support for remote Home access. The user's current Shortcuts bridge remains a LAN HTTP device and is not made into a Home accessory by adding a hub.

## Open items for V1

- Add the actual `Main_program.ino` source.
- Add the `Bluetooth_device_scanner.ino` source used to discover the controller.
- Confirm `/on` on the actual Lotus controller after the explicit ON-frame change.
- Record the BLE characteristic's write properties and the successful write mode.
- Reserve a DHCP address for the ESP32 or document how to find its current address.
- Decide on a license and whether the repository will be public or private.
