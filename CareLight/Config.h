/*
  Config.h

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

#ifndef CONFIG_h
#define CONFIG_h

#include <Arduino.h>
#include <Preferences.h>

class Config
{
private:
  Preferences preferences;
  String readLine(void);
  //KeyReport _keyReport;
  //const uint8_t *_asciimap;
  //void sendReport(KeyReport* keys);
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
  void SetDefaults(void);
  bool IsSet(void);
  void Update(void);
  void Read(void);
  void Print(void);
  uint16_t StoreInt(const char* prompt, const char* key, uint16_t defaultValue, uint16_t minValue, uint16_t maxValue);
  uint8_t StoreColor(const char* prompt, const char* key, uint8_t defaultValue, uint8_t minValue, uint8_t maxValue);
  void PromptAndReadLineDefault(const char* prompt, const char* key, const char* defaultValue);
  String PromptAndReadLine(const char* prompt);


  //void begin(const uint8_t *layout = KeyboardLayout_en_US);
  //void end(void);
  //size_t write(uint8_t k);
  //size_t write(const uint8_t *buffer, size_t size);
  //size_t press(uint8_t k);
  //size_t release(uint8_t k);
  //void releaseAll(void);
};

#endif