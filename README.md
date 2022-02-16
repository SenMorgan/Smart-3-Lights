# Smart 3 Lights Controller

Smart Controller for 3 Lights with MQTT and WEB configuration portal. <br>
The project was created in PlatformIO 16.11.2021

<br>


[![Build with PlatformIO](https://img.shields.io/badge/Build%20with-PlatformIO-orange)](https://platformio.org/)

[![ESP8266](https://img.shields.io/badge/ESP-8266-000000.svg?longCache=true&style=flat&colorA=AA101F)](https://www.espressif.com/en/products/socs/esp8266)

[![ESP32](https://img.shields.io/badge/ESP-32-000000.svg?longCache=true&style=flat&colorA=AA101F)](https://www.espressif.com/en/products/socs/esp32)

[![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg)](https://opensource.org/licenses/MIT)

<br>

## Wiring Schematic

<img src="https://media3.giphy.com/media/TLeLKUdIc1tvAxb7ab/source.gif" width="400" height="250" />

<br>

## Home Assistant YAML configuration
```yaml
light:
  - platform: mqtt
    name: "Smart 3 Lights"
    state_topic: "/smart-3-lights/state"
    command_topic: "/smart-3-lights/set"
    payload_on: "1"
    payload_off: "0"
    availability_topic: "/smart-3-lights/availability"
    payload_available: "online"
    payload_not_available: "offline"
    optimistic: false
```

<br>

## Notes

If you want to rebuild this project in VS Code with PlatformIO, you need to manually download [WiFiManager library v2.0.4-beta](https://github.com/tzapu/WiFiManager) and put it in `lib/WiFiManager/`.

<br>

## Dependencies
MQTT library v2.8 https://github.com/knolleary/pubsubclient

<br>

## Copyright

Copyright (c) 2021 Sen Morgan. Licensed under the MIT license, see LICENSE.md