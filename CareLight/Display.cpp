/*
 Display.cpp is a part of CareLight program
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

#include <Arduino.h>
#include <FastLED.h>
#include "Display.h" //ok
#include "Config.h" //ok

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

Display::Display(Config& cfg) : config(cfg) 
{ }

void Display::Init(void)
{
  Serial.print("Initializing LED display... ");
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  Serial.println("Done");
}

void Display::SetBrightness(uint8_t brightness) {
  FastLED.setBrightness( brightness );
}

void Display::Arrow(ArrowType type, uint8_t hue) {
  byte* mask = GetArrowMask(type);
  CRGB color = GetArrowColor(hue);
  DisplayMask(mask, color);
}

CRGB Display::GetArrowColor(uint8_t hue) {
  return CHSV( hue, 255, 255);
}

byte* Display::GetArrowMask(ArrowType type) {
  if (type == UP_TRIPLE) {
    return maskTripleUp;
  } else if (type == UP_DOUBLE) {
    return maskDoubleUp;
  } else if (type == UP) {
    return maskUp;
  } else if (type == STABLE) {
    return maskStable;
  } else if (type == DOWN) {
    return maskDown;
  } else if (type == DOWN_DOUBLE) {
    return maskDoubleDown;
  } else if (type == DOWN_TRIPLE) {
    return maskTripleDown;
  } else if (type == TOO_HIGH) {
    return maskHi;
  } else if (type == TOO_LOW) {
    return maskLow;
  } else {
    return maskEmpty;
  } 
}

void Display::Rainbow(void) {
  for (int i=0; i<NUM_LEDS; i++) {
    leds[i] = CHSV( (i/64.0) * 255, 255, 255); 
  }
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
}

void Display::Logo(void) {
  for (int i=0; i<NUM_LEDS; i++) {
    if (maskLogo[i] == 1) {
      leds[i] = CHSV( map(i, 0, 63, 115, 240), 255, 255); 
    } else {
      leds[i] = ledOff;
    }
  }
}

void Display::RedError(void) {
  DisplayMask(maskError, ledRed);
}

void Display::YellowError(void) {
  DisplayMask(maskError, ledYellow);
}

void Display::BlueError(void) {
  DisplayMask(maskError, ledBlue);
}

void Display::GreenError(void) {
  DisplayMask(maskError, ledGreen);
}

void Display::DisplayMask(byte* mask, CRGB color) 
{
  for (int i=0; i<NUM_LEDS; i++) 
  {
    if (mask[i] == 1) 
    {
      leds[i] = color;
    } 
    else 
    {
      leds[i] = ledOff;
    }
  }
}

void Display::FillColor(CRGB currentColor) 
{
  for (int i=0; i<NUM_LEDS; i++) 
  {
    leds[i] = currentColor;
  }
}

void Display::ClearDisplay(void) 
{
  FillColor(ledOff);
}

void Display::Update(void)
{
  FastLED.show();
}
