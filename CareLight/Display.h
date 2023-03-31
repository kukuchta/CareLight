/*
  Display.h

  Copyright (c) 2023, Jakub Kuchta

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef DISPLAY_h
#define DISPLAY_h

#define NUM_LEDS 64
#define DATA_PIN 12
//#define DATA_PIN 13

#include <Arduino.h>
#include <FastLED.h>
#include "Config.h"

class Display
{
private:
  Config &config;
  CRGB leds[NUM_LEDS];
  CRGB GetColorFromSg(int sg);
  byte* GetArrowFromTrend(const String& trend);  
  void Error(CRGB currentColor);
  void Arrow(byte* mask, int currentSg);
public:
  Display(Config &cfg);
  void Init(void);
  void SetBrightness(uint8_t brightness);
  void Rainbow(void);
  void FullScale(void);
  void Logo(void);
  void RedError(void);
  void YellowError(void);
  void BlueError(void);
  void GreenError(void);
  void TripleUpArrow(int currentSg);
  void DoubleUpArrow(int currentSg);
  void UpArrow(int currentSg);
  void StableArrow(int currentSg);
  void DownArrow(int currentSg);
  void DoubleDownArrow(int currentSg);
  void TripleDownArrow(int currentSg);
  void TooHighArrow(int currentSg);
  void TooLowArrow(int currentSg);
  void ArrowFromTrend(const String& currentTrend, int currentSg);
  void FillColor(CRGB currentColor);
  void ClearDisplay(void);
};

#endif