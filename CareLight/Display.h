/*
 Display.h is a part of CareLight program
 Copyright 2023 Jakub Kuchta

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef DISPLAY_h
#define DISPLAY_h

#define NUM_LEDS 64
#define DATA_PIN 12
//#define DATA_PIN 13

#include <FastLED.h>

enum ArrowType {
  NONE,
  UP_TRIPLE,
  UP_DOUBLE,
  UP,
  STABLE,
  DOWN,
  DOWN_DOUBLE,
  DOWN_TRIPLE,
  TOO_HIGH,
  TOO_LOW
};

class Config;

class Display
{
public:
  Display(Config& cfg);
  void Init(void);
  void SetBrightness(uint8_t brightness);
  void Rainbow(void);
  void FullScale(void);
  void Logo(void);
  void RedError(void);
  void YellowError(void);
  void BlueError(void);
  void GreenError(void);
  void Arrow(ArrowType type, uint8_t hue);
  void FillColor(CRGB currentColor);
  void ClearDisplay(void);
  void Update(void);
private:
  Config& config; 
  CRGB leds[NUM_LEDS];
  CRGB GetArrowColor(uint8_t hue);
  byte* GetArrowMask(ArrowType type);  
  void DisplayMask(byte* mask, CRGB color);
};

#endif