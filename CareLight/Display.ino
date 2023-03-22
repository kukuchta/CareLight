/////////////////////////////////////////////////////////////////////////////////
// Display functions ////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

#include <Arduino.h>

#include <FastLED.h>

#define NUM_LEDS 64
#define DATA_PIN 12
//#define DATA_PIN 13

CRGB leds[NUM_LEDS];
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


void InitDisplay() {
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
}

void SetDisplayBrightness(uint8_t brightness) {
  FastLED.setBrightness( brightness );
  FastLED.show();
}

CRGB GetColorFromSg(int sg) {
  uint8_t* color = GetColors();
  uint16_t* treshold = GetTresholds();
  CRGB newColor;
  if (sg >= 40 && sg <= treshold[0]) {  
    newColor = CHSV( color[0], 255, 255);
  } else if (sg <= treshold[1]) {  
    newColor = CHSV( color[1], 255, 255);
  } else if (sg <= treshold[2]) {  
    newColor = CHSV( color[2], 255, 255);
  } else if (sg <= treshold[3]) {  
    newColor = CHSV( color[3], 255, 255);
  } else if (sg <= treshold[4]) {  
    newColor = CHSV( color[4], 255, 255);
  } else if (sg <= treshold[5]) {  
    newColor = CHSV( color[5], 255, 255);
  } else if (sg <= treshold[6]) {  
    newColor = CHSV( color[6], 255, 255);
  } else if (sg <= 400) {  
    newColor = CHSV( color[7], 255, 255);
  } else {
    newColor = ledOff;
  }
  return newColor;
}

byte* GetArrowFromTrend(const String& trend) {
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

void DisplayRainbow() {
  for (int i=0; i<NUM_LEDS; i++) {
    leds[i] = CHSV( (i/64.0) * 255, 255, 255); 
  }
  FastLED.show();
}

void DisplayFullScale() {
  uint8_t* color = GetColors();
  for (int i=0; i<NUM_LEDS; i+=8) {
      leds[i+0] = CHSV( color[0], 255, 255); 
      leds[i+1] = CHSV( color[1], 255, 255);
      leds[i+2] = CHSV( color[2], 255, 255);
      leds[i+3] = CHSV( color[3], 255, 255);
      leds[i+4] = CHSV( color[4], 255, 255);
      leds[i+5] = CHSV( color[5], 255, 255);
      leds[i+6] = CHSV( color[6], 255, 255);
      leds[i+7] = CHSV( color[7], 255, 255);
  }
  FastLED.show();
}

void DisplayLogo() {
  for (int i=0; i<NUM_LEDS; i++) {
    if (maskLogo[i] == 1) {
      leds[i] = CHSV( map(i, 0, 63, 115, 240), 255, 255); 
    } else {
      leds[i] = ledOff;
    }
  }
  FastLED.show();
}

void DisplayError(CRGB currentColor) {
  for (int i=0; i<NUM_LEDS; i++) {
    if (maskError[i] == 1) {
      leds[i] = currentColor; 
    } else {
      leds[i] = ledOff;
    }
  }
  FastLED.show();
}

void DisplayRedError() {
  DisplayError(ledRed);
}

void DisplayYellowError() {
  DisplayError(ledYellow);
}

void DisplayBlueError() {
  DisplayError(ledBlue);
}

void DisplayGreenError() {
  DisplayError(ledGreen);
}

void DisplayArrow(byte* mask, int currentSg) {
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

void DisplayTripleUpArrow(int currentSg) {
  DisplayArrow(maskTripleUp, currentSg);
}

void DisplayDoubleUpArrow(int currentSg) {
  DisplayArrow(maskDoubleUp, currentSg);
}

void DisplayUpArrow(int currentSg) {
  DisplayArrow(maskUp, currentSg);
}

void DisplayStableArrow(int currentSg) {
  DisplayArrow(maskStable, currentSg);
}

void DisplayDownArrow(int currentSg) {
  DisplayArrow(maskDown, currentSg);
}

void DisplayDoubleDownArrow(int currentSg) {
  DisplayArrow(maskDoubleDown, currentSg);
}

void DisplayTripleDownArrow(int currentSg) {
  DisplayArrow(maskTripleDown, currentSg);
}

void DisplayTooHighArrow(int currentSg) {
  DisplayArrow(maskHi, currentSg);
} 

void DisplayTooLowArrow(int currentSg) {
  DisplayArrow(maskLow, currentSg);
}

void DisplayArrowFromTrend(const String& currentTrend, int currentSg) {
  DisplayArrow(GetArrowFromTrend(currentTrend), currentSg);
}

void DisplayColor(CRGB currentColor) {
  for (int i=0; i<NUM_LEDS; i++) {
    leds[i] = currentColor;
  }
  FastLED.show();
}

void ClearDisplay() {
  DisplayColor(ledOff);
}


