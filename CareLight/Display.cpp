/*
  Display.cpp

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

#include <Arduino.h>
#include <FastLED.h>
#include "Display.h"
#include "Config.h"

CRGB ledOff = CHSV( 0, 0, 0);
CRGB ledRed = CHSV( 0, 255, 255);
CRGB ledYellow = CHSV( 40, 255, 255);
CRGB ledBlue = CHSV( 160, 255, 255);
CRGB ledGreen = CHSV( 96, 255, 255);

byte maskUp[NUM_LEDS] = {1,1,1,1,1,1,1,1,
                         1,1,1,1,1,1,1,1,
                         0,0,0,0,0,0,1,1,
                         0,0,0,0,0,0,1,1,
                         0,0,0,0,0,0,1,1,
                         0,0,0,0,0,0,1,1,
                         0,0,0,0,0,0,1,1,
                         0,0,0,0,0,0,1,1,};

byte maskDoubleUp[NUM_LEDS] = {1,1,1,1,1,1,1,1,
                               1,1,1,1,1,1,1,1,
                               0,0,0,0,0,0,1,1,
                               1,1,1,1,1,0,1,1,
                               1,1,1,1,1,0,1,1,
                               0,0,0,1,1,0,1,1,
                               0,0,0,1,1,0,1,1,
                               0,0,0,1,1,0,1,1,};

byte maskTripleUp[NUM_LEDS] = {1,1,1,1,1,1,1,1,
                               1,1,1,1,1,1,1,1,
                               0,0,0,0,0,0,1,1,
                               1,1,1,1,1,0,1,1,
                               1,1,1,1,1,0,1,1,
                               0,0,0,1,1,0,1,1,
                               1,1,0,1,1,0,1,1,
                               1,1,0,1,1,0,1,1,};

byte maskStable[NUM_LEDS] = {0,0,0,0,0,0,1,1,
                             0,0,0,0,0,0,1,1,
                             0,0,0,0,0,0,1,1,
                             0,0,0,0,0,0,1,1,
                             0,0,0,0,0,0,1,1,
                             0,0,0,0,0,0,1,1,
                             1,1,1,1,1,1,1,1,
                             1,1,1,1,1,1,1,1,};

byte maskDown[NUM_LEDS] = {1,1,0,0,0,0,0,0,
                           1,1,0,0,0,0,0,0,
                           1,1,0,0,0,0,0,0,
                           1,1,0,0,0,0,0,0,
                           1,1,0,0,0,0,0,0,
                           1,1,0,0,0,0,0,0,
                           1,1,1,1,1,1,1,1,
                           1,1,1,1,1,1,1,1,};

byte maskDoubleDown[NUM_LEDS] = {1,1,0,1,1,0,0,0,
                                 1,1,0,1,1,0,0,0,
                                 1,1,0,1,1,0,0,0,
                                 1,1,0,1,1,1,1,1,
                                 1,1,0,1,1,1,1,1,
                                 1,1,0,0,0,0,0,0,
                                 1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,};

byte maskTripleDown[NUM_LEDS] = {1,1,0,1,1,0,1,1,
                                 1,1,0,1,1,0,1,1,
                                 1,1,0,1,1,0,0,0,
                                 1,1,0,1,1,1,1,1,
                                 1,1,0,1,1,1,1,1,
                                 1,1,0,0,0,0,0,0,
                                 1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,};

byte maskError[NUM_LEDS] = {0,0,0,0,0,0,1,1,
                            0,0,0,0,0,1,1,1,
                            0,0,0,0,1,1,1,0,
                            0,0,0,1,1,1,0,0,
                            0,0,0,1,1,0,0,0,
                            0,0,0,0,0,0,0,0,
                            1,1,0,0,0,0,0,0,
                            1,1,0,0,0,0,0,0,};

byte maskLow[NUM_LEDS] = {1,0,0,0,0,0,1,1,
                          1,0,0,0,0,1,1,1,
                          1,0,0,0,1,1,1,0,
                          1,0,0,0,1,1,0,0,
                          1,0,1,1,0,0,0,0,
                          1,0,1,1,0,0,0,0,
                          1,0,0,0,0,0,0,0,
                          1,1,1,1,1,1,1,1,};

byte maskHi[NUM_LEDS] = {1,1,1,1,1,1,1,1,
                         0,0,0,0,0,0,0,1,
                         0,0,0,0,1,1,0,1,
                         0,0,0,1,1,1,0,1,
                         0,0,1,1,1,0,0,1,
                         0,0,1,1,0,0,0,1,
                         1,1,0,0,0,0,0,1,
                         1,1,0,0,0,0,0,1,};

byte maskEmpty[NUM_LEDS] = {0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,};

byte maskLogo[NUM_LEDS] =  {0,0,1,1,1,1,0,0,
                            0,1,1,1,1,1,1,0,
                            1,1,1,0,0,1,1,1,
                            1,1,0,0,0,0,1,1,
                            1,1,0,0,0,0,1,1,
                            1,1,1,0,0,1,1,1,
                            0,1,1,1,1,1,1,0,
                            0,0,1,1,1,1,0,0,};

Display::Display(Config &cfg) : config(cfg) 
{ }

void Display::Init(void)
{
  Serial.print("Initializing LED display... ");
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  Serial.println("Done");
}

void Display::SetBrightness(uint8_t brightness) {
  FastLED.setBrightness( brightness );
  FastLED.show();
}

CRGB Display::GetColorFromSg(int sg) {
  CRGB newColor;
  if (sg >= 40 && sg <= config.treshold[0]) {  
    newColor = CHSV( config.color[0], 255, 255);
  } else if (sg <= config.treshold[1]) {  
    newColor = CHSV( config.color[1], 255, 255);
  } else if (sg <= config.treshold[2]) {  
    newColor = CHSV( config.color[2], 255, 255);
  } else if (sg <= config.treshold[3]) {  
    newColor = CHSV( config.color[3], 255, 255);
  } else if (sg <= config.treshold[4]) {  
    newColor = CHSV( config.color[4], 255, 255);
  } else if (sg <= config.treshold[5]) {  
    newColor = CHSV( config.color[5], 255, 255);
  } else if (sg <= config.treshold[6]) {  
    newColor = CHSV( config.color[6], 255, 255);
  } else if (sg <= 400) {  
    newColor = CHSV( config.color[7], 255, 255);
  } else {
    newColor = ledOff;
  }
  return newColor;
}

byte* Display::GetArrowFromTrend(const String& trend) {
  String trendString = String(trend);
  if (trendString.equals("UP_TRIPLE")) {
    return maskTripleUp;
  } else if (trendString.equals("UP_DOUBLE")) {
    return maskDoubleUp;
  } else if (trendString.equals("UP")) {
    return maskUp;
  } else if (trendString.equals("NONE")) {
    return maskStable;
  } else if (trendString.equals("DOWN")) {
    return maskDown;
  } else if (trendString.equals("DOWN_DOUBLE")) {
    return maskDoubleDown;
  } else if (trendString.equals("DOWN_TRIPLE")) {
    return maskTripleDown;
  } else {
    return maskEmpty;
  } 
}

void Display::Rainbow(void) {
  for (int i=0; i<NUM_LEDS; i++) {
    leds[i] = CHSV( (i/64.0) * 255, 255, 255); 
  }
  FastLED.show();
}

void Display::FullScale() {
  for (int i=0; i<NUM_LEDS; i+=8) {
      leds[i+0] = CHSV( config.color[0], 255, 255); 
      leds[i+1] = CHSV( config.color[1], 255, 255);
      leds[i+2] = CHSV( config.color[2], 255, 255);
      leds[i+3] = CHSV( config.color[3], 255, 255);
      leds[i+4] = CHSV( config.color[4], 255, 255);
      leds[i+5] = CHSV( config.color[5], 255, 255);
      leds[i+6] = CHSV( config.color[6], 255, 255);
      leds[i+7] = CHSV( config.color[7], 255, 255);
  }
  FastLED.show();
}

void Display::Logo(void) {
  for (int i=0; i<NUM_LEDS; i++) {
    if (maskLogo[i] == 1) {
      leds[i] = CHSV( map(i, 0, 63, 115, 240), 255, 255); 
    } else {
      leds[i] = ledOff;
    }
  }
  FastLED.show();
}

void Display::Error(CRGB currentColor) {
  for (int i=0; i<NUM_LEDS; i++) {
    if (maskError[i] == 1) {
      leds[i] = currentColor; 
    } else {
      leds[i] = ledOff;
    }
  }
  FastLED.show();
}

void Display::RedError(void) {
  Error(ledRed);
}

void Display::YellowError(void) {
  Error(ledYellow);
}

void Display::BlueError(void) {
  Error(ledBlue);
}

void Display::GreenError(void) {
  Error(ledGreen);
}

void Display::Arrow(byte* mask, int currentSg) {
  CRGB color = GetColorFromSg(currentSg);
  for (int i=0; i<NUM_LEDS; i++) {
    if (mask[i] == 1) {
      leds[i] = color;
    } else {
      leds[i] = ledOff;
    }
  }
  FastLED.show();
}

void Display::TripleUpArrow(int currentSg) {
  Arrow(maskTripleUp, currentSg);
}

void Display::DoubleUpArrow(int currentSg) {
  Arrow(maskDoubleUp, currentSg);
}

void Display::UpArrow(int currentSg) {
  Arrow(maskUp, currentSg);
}

void Display::StableArrow(int currentSg) {
  Arrow(maskStable, currentSg);
}

void Display::DownArrow(int currentSg) {
  Arrow(maskDown, currentSg);
}

void Display::DoubleDownArrow(int currentSg) {
  Arrow(maskDoubleDown, currentSg);
}

void Display::TripleDownArrow(int currentSg) {
  Arrow(maskTripleDown, currentSg);
}

void Display::TooHighArrow(int currentSg) {
  Arrow(maskHi, currentSg);
} 

void Display::TooLowArrow(int currentSg) {
  Arrow(maskLow, currentSg);
}

void Display::ArrowFromTrend(const String& currentTrend, int currentSg) {
  Arrow(GetArrowFromTrend(currentTrend), currentSg);
}

void Display::FillColor(CRGB currentColor) {
  for (int i=0; i<NUM_LEDS; i++) {
    leds[i] = currentColor;
  }
  FastLED.show();
}

void Display::ClearDisplay(void) {
  FillColor(ledOff);
}
