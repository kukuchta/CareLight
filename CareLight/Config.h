/*
 Config.h is a part of CareLight program
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

#ifndef CONFIG_h
#define CONFIG_h

#include <FastLED.h>
#include <Preferences.h>

class Display;

class Config
{
public:
  String ssid;  
  String password;
  String careLinkUser;
  String careLinkPassword;
  String careLinkCountry;
  uint8_t brightness;
  uint8_t color[8];
  uint16_t treshold[7];
  Config(void);  
  void Init(Display& display);
private:
  Preferences preferences;
  String ReadLine(void);
  void SetDefaults(void);
  bool IsSet(void);
  void Update(Display& display);
  void Read(void);
  void Print(void);
  uint16_t StoreInt(const char* prompt, const char* key, uint16_t defaultValue, uint16_t minValue, uint16_t maxValue);
  uint8_t StoreColor(Display& display, const char* prompt, const char* key, uint8_t defaultValue, uint8_t minValue, uint8_t maxValue);
  void PromptAndReadLineDefault(const char* prompt, const char* key, const char* defaultValue);
  String PromptAndReadLine(const char* prompt);
};

#endif