# Hardware ([wersja polska](./Hardware_pl.md))

## Main board

The program is intended to run on ESP32 series WiFi/BT chip. There are other similar series available on the market like ESP32-S2, -S3, -C3 or -C6 but they differ from plain ESP32 in amount of RAM memory, peripherials etc. Currently the plain ESP32 is only tested solution. 

There are several development boards available that include this chip, like ESP-WROOM-32, Lolin32 Lite, Wemos D1 R32, Wemos D1 Mini ESP-32, ESP32-CAM etc. The only real requirement for the board is that there are 3 pins available for easy connection of the LED array:
* GND pin
* Output pin - GPIO12 as hardcoded in the sources, but after change it can be any GPIO pin
* +5V pin - this is the tricky one. Not every board has dedicated pin that can be used as USB +5V voltage source.

Many boards have only 3.3V output pins as this is the voltage that ESP32 works with. Such voltage will be not enough to power on the LED array.
* ESP-WROOM-32 (find [here](https://allegro.pl/listing?string=esp-wroom-32)) has VIN pin that fits the purpose, 
* Lolin32 Lite (find [here](https://allegro.pl/listing?string=lolin32%20lite)) does not have it, but after slight modification with knife and soldering iron (see below) it can be done. 

### Lolin32 modification

This particular board is worth trying because its smaller than ESP-WROOM-32, much cheaper and provides support for Li-Poly battery charging without any additional hardware (CareLight can be truly wireless for some time :).

To turn pin GPIO 14 into +4.5V voltage source:
1. Cut the thin path leading to pin 14 with sharp knife (we disconnect it from the ESP32 chip)
2. Solder a bridge from pin 14 to input pin of the onboard 3.3V voltage regulator (we connect pin 14 to the point where is available voltage from both USB and Li-Poly)

This way we have three neighbouring pins - G, 12 and 14 (turned into +V) - ready to connect to the LED array.

## LED array board

Program is intended to display data on the array of 64 WS2812B RGB LEDs shaped into 8x8 pattern. LEDs should be connected in a way where LED index in every row increases from left to right, and last LED in a row is connected with first LED in next row (no alternating rows). It is most common LED layout on cheap modules available on allegro (find [here](https://allegro.pl/listing?string=8x8%20ws2812b)) or china markets ([here](https://aliexpress.com/w/wholesale-8x8-ws2812b.html)).

Scrambled arrows or switched directions are common effects of incompatible LED layout on board but it can be fixed in the code (to be done in the future).

Usually there are two sets of 3 pins on the LED boards. The one we need consists of positive voltage (VCC/+5V/V+), ground (G/GND/V-) and data input (DIN/IN/INPUT). Other set with data output (DOUT/OUT/OUTPUT) is used to connect more LEDs in a chain (not used here).

## Connection cables

Only 3 female to female DuPont/goldpin wires are needed. Of course everything could be soldered together instead for better quality.
