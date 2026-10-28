# iPhone Shortcuts

The bridge and iPhone need to be on the same home network for the local URLs below to work.

## Create an On shortcut

1. In Shortcuts, tap **+** and name the shortcut `Lights On`.
2. Add the **URL** action with `http://ESP32-IP/on`.
3. Add **Get Contents of URL** and set the method to **GET**.
4. Run it once and allow local network access if iOS prompts.

## Create an Off shortcut

Repeat the steps with the name `Lights Off` and URL `http://ESP32-IP/off`.

## Colour shortcuts

Use a URL such as `http://ESP32-IP/color?hex=ff0000` for red, followed by **Get Contents of URL** set to **GET**. RGB can also be specified, for example `http://ESP32-IP/color?r=255&g=0&b=80`.

## Sunrise and sunset

Create two **Personal Automations** with **Time of Day** triggers: daily at sunset, run `Lights On`; daily at sunrise, run `Lights Off`. Set each automation to run without asking. Shortcuts automations are tied to the iPhone that created them.

If the phone is away from home, the local IP request will fail unless a VPN connects it to the home network. The automation trigger can still fire, but the ESP32 must be reachable when the action runs.

## Remote access

The current HTTP bridge has no authentication. Do not port-forward it to the public internet. Use a VPN such as a router VPN or a private-network setup with a subnet router on an always-on home device. Then keep using the ESP32's LAN IP from the Shortcut.
