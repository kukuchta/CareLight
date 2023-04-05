# CareLight
CareLight is a simple color LED indicator over WiFi for people with diabetes using Medtronic's 740g/780g insulin pumps.
It connects with Medtronic's CareLink cloud and downloads current glucose level data.

## Disclaimer and Warning

This project is intended for educational and informational purposes only. It is not approved by any medical authority (FDA etc.). It is not properly tested and must not be used for making any kind of decisions regarding medical treatment. It is neither affiliated with nor endorsed by Medtronic, and may violate their Terms of Service. Use of this code is without warranty or any kind of support.

## License

This project is licensed under terms of [GNU General Public License v3.0](./LICENSE.md)

## Features
* Displays arrow indications with regard to current glucose level and trend:
  * There are 8 differend colors indicating glucose level and 7 thresholds between them
  * Both colors and thresholds are user-defined
  * There are easy to distinguish arrow types for glucose trend:
    * over range  (>400 mg/dL)
    * triple up   (>20 mg/dL/5min)
    * double up   (12-19 mg/dL/5min)
    * up          (3-11 mg/dL/5min)
    * steady      (0-2 mg/dL/5min)
    * down        (3-11 mg/dL/5min)
    * double down (12-19 mg/dL/5min)
    * triple down (>20 mg/dL/5min)
    * under range (<40 mg/dL)
* Displays indications about common connection problems:
  * Yellow '!' for WiFi connection problems between CareLight and Wifi network
  * Green '!' for state between successful WiFi connection and first data arrival
  * Red '!' for problems with internet connection or CareLink cloud availability
  * Blue '!' for problems connection problems between Medtronic pump and the CareLink cloud 
* Easy configuration through any terminal application over USB cable

## Hardware requirements
* More informations about used hardware can be found [here](./Documents/Hardware.md).
* Controller board (any following):
  * ESP-WROOM-32 board
  * Wemos Lolin32 Lite (with hardware modification)
* Display board:
  * 8x8 WS2812B RGB LED array board

## Software requirements for programming and configuration:
* Arduino IDE >= 2.0.4 with:
  * ESP32 Arduino Core board support >= v2.0.4
  * FastLED library by Daniel Garcia >= v3.5
  * ArduinoJson library by Benoit Blanchon >= v6.21.1
  * StreamUtils library by Benoit Blanchon >= v1.7.3 (to be removed)
* CH340G USB-to-serial Drivers 

## Setup and manual
* Setup, programming manual process is explained [here](./Documents/Setup.md).
* Short manual can be found [here](./Documents/Manual.md).
