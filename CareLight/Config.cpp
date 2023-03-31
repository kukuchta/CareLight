/*
  Config.cpp

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
#include "Config.h"

Config::Config(void)
{ }

void Config::SetDefaults(void)
{
  ssid = "ssid";  
  password = "password";
  careLinkUser = "user";
  careLinkPassword = "password";
  careLinkCountry = "pl";
  brightness = 25;
  
  treshold[0] = 54;
  treshold[1] = 69;
  treshold[2] = 109;
  treshold[3] = 139;
  treshold[4] = 179;
  treshold[5] = 209;
  treshold[6] = 249;
    
  color[0] = 192;
  color[1] = 160;
  color[2] = 115;
  color[3] = 96;
  color[4] = 70;
  color[5] = 28;
  color[6] = 12;
  color[7] = 1;
}

bool Config::IsSet(void) {
  preferences.begin("config", false);
  bool result = preferences.isKey("WIFI_SSID");
  preferences.end();
  return result;
}

void Config::Update(void){
  preferences.begin("config", false);
  preferences.clear();
  while (Serial.available() > 0) {
    Serial.read();
  }
  PromptAndReadLineDefault("Wifi network name (ssid)", "WIFI_SSID", ssid.c_str());
  PromptAndReadLineDefault("Wifi network password", "WIFI_PASS", password.c_str());
  PromptAndReadLineDefault("CareLink user name", "CARELINK_USER", careLinkUser.c_str());
  PromptAndReadLineDefault("CareLink password", "CARELINK_PASS", careLinkPassword.c_str());
  PromptAndReadLineDefault("CareLink country code", "CARELINK_CNTRY", careLinkCountry.c_str());

  brightness = StoreInt("LED brightness", "BRIGHTNESS", brightness, 1, 255);
  //FastLED.setBrightness( brightness );  TODO
  
  Serial.println("\nSetup 8 glucose ranges, each with different color.");
  Serial.println("Possible hue values are 1-255, where: \n 1 - Red,\n 12 - Dark orange,\n 22 - Orange,\n 40 - Yellow,\n 96 - Green,\n 128 - Cyan,\n 160 - Blue,\n 192 - Violet,\n 255 - Red\n");

  Serial.println("Lowest range from 40 mg/dl - set maximum value.");
  treshold[0] = StoreInt("  Treshold 1", "TRESHOLD_1", treshold[0], 41, 393);
  Serial.println("Lowest range from 40 to " + String(treshold[0]) + " mg/dl - set color hue.");
  color[0] = StoreColor("  Color 1", "COLOR_1", color[0], 1, 255);

  Serial.println("Range 2 from " + String(treshold[0] + 1) + " mg/dl - set maximum value.");
  treshold[1] = StoreInt("  Treshold 2", "TRESHOLD_2", treshold[1], (treshold[0] + 1), 394);
  Serial.println("Range 2 from " + String(treshold[0] + 1) + " to " + String(treshold[1]) + " mg/dl - set color hue.");
  color[1] = StoreColor("  Color 2", "COLOR_2", color[1], 1, 255);

  Serial.println("Range 3 from " + String(treshold[1] + 1) + " mg/dl - set maximum value.");
  treshold[2] = StoreInt("  Treshold 3", "TRESHOLD_3", treshold[2], (treshold[1] + 1), 395);
  Serial.println("Range 3 from " + String(treshold[1] + 1) + " to " + String(treshold[2]) + " mg/dl - set color hue.");
  color[2] = StoreColor("  Color 3", "COLOR_3", color[2], 1, 255);

  Serial.println("Range 4 from " + String(treshold[2] + 1) + " mg/dl - set maximum value.");
  treshold[3] = StoreInt("  Treshold 4", "TRESHOLD_4", treshold[3], (treshold[2] + 1), 396);
  Serial.println("Range 4 from " + String(treshold[2] + 1) + " to " + String(treshold[3]) + " mg/dl - set color hue.");
  color[3] = StoreColor("  Color 4", "COLOR_4", color[3], 1, 255);

  Serial.println("Range 5 from " + String(treshold[3] + 1) + " mg/dl - set maximum value.");
  treshold[4] = StoreInt("  Treshold 5", "TRESHOLD_5", treshold[4], (treshold[3] + 1), 397);
  Serial.println("Range 5 from " + String(treshold[3] + 1) + " to " + String(treshold[4]) + " mg/dl - set color hue.");
  color[4] = StoreColor("  Color 5", "COLOR_5", color[4], 1, 255);

  Serial.println("Range 6 from " + String(treshold[4] + 1) + " mg/dl - set maximum value.");
  treshold[5] = StoreInt("  Treshold 6", "TRESHOLD_6", treshold[5], (treshold[4] + 1), 398);
  Serial.println("Range 6 from " + String(treshold[4] + 1) + " to " + String(treshold[5]) + " mg/dl - set color hue.");
  color[5] = StoreColor("  Color 6", "COLOR_6", color[5], 1, 255);

  Serial.println("Range 7 from " + String(treshold[5] + 1) + " mg/dl - set maximum value.");
  treshold[6] = StoreInt("  Treshold 7", "TRESHOLD_7", treshold[6], (treshold[5] + 1), 399);
  Serial.println("Range 7 from " + String(treshold[5] + 1) + " to " + String(treshold[6]) + " mg/dl - set color hue.");
  color[6] = StoreColor("  Color 7", "COLOR_7", color[6], 1, 255);

  Serial.println("Highest range from " + String(treshold[6] + 1) + " to 400 mg/dl - set color hue.");
  color[7] = StoreColor("  Color 8", "COLOR_8", color[7], 1, 255);
  //ClearDisplay(); TODO
  preferences.end();
}

void Config::Read(void){ 
  preferences.begin("config", false);

  ssid = preferences.getString("WIFI_SSID");  
  password = preferences.getString("WIFI_PASS");
  careLinkUser = preferences.getString("CARELINK_USER");
  careLinkPassword = preferences.getString("CARELINK_PASS");
  careLinkCountry = preferences.getString("CARELINK_CNTRY");
  brightness = preferences.getUShort("BRIGHTNESS");
  
  treshold[0] = preferences.getUShort("TRESHOLD_1");
  treshold[1] = preferences.getUShort("TRESHOLD_2");
  treshold[2] = preferences.getUShort("TRESHOLD_3");
  treshold[3] = preferences.getUShort("TRESHOLD_4");
  treshold[4] = preferences.getUShort("TRESHOLD_5");
  treshold[5] = preferences.getUShort("TRESHOLD_6");
  treshold[6] = preferences.getUShort("TRESHOLD_7");
    
  color[0] = preferences.getUChar("COLOR_1");
  color[1] = preferences.getUChar("COLOR_2");
  color[2] = preferences.getUChar("COLOR_3");
  color[3] = preferences.getUChar("COLOR_4");
  color[4] = preferences.getUChar("COLOR_5");
  color[5] = preferences.getUChar("COLOR_6");
  color[6] = preferences.getUChar("COLOR_7");
  color[7] = preferences.getUChar("COLOR_8");

  preferences.end();
}

void Config::Print(void){ 
  Serial.print("  WiFi: ");
  Serial.println(ssid);
  Serial.print("  CareLink user: ");
  Serial.println(careLinkUser);
  Serial.print("  CareLink country: ");
  Serial.println(careLinkCountry);
  Serial.print("  LED brightness: ");
  Serial.println(brightness);
  
  for(int i = 0; i<7; i++) {
    Serial.print("  Treshold ");
    Serial.print(i+1);
    Serial.print(": ");    
    Serial.println(treshold[i]);
  }
  
  for(int i = 0; i<8; i++) {
    Serial.print("  Color ");
    Serial.print(i+1);
    Serial.print(": ");
    Serial.println(color[i]);
  } 
}

uint16_t Config::StoreInt(const char* prompt, const char* key, uint16_t defaultValue, uint16_t minValue, uint16_t maxValue) {
  bool resultNotOk = false;
  uint16_t value = 0;
  
  do {
    Serial.print(prompt);
    Serial.print(" (" + String(minValue) + "-" + String(maxValue) + ") [Enter for '" + String(defaultValue) + "']: ");
    String s = readLine();
    value = 0;
    if (s != ""){
      value = atoi(s.c_str());  
    } else {
      value = defaultValue;
    }

    resultNotOk = (value == 0 || value < minValue || value > maxValue);
    if (resultNotOk) {
      Serial.println("Error, value outside range.");
    } else {
      Serial.println(value);
      preferences.putUShort(key, value);
    }
  } while (resultNotOk);

  return value;
}

uint8_t Config::StoreColor(const char* prompt, const char* key, uint8_t defaultValue, uint8_t minValue, uint8_t maxValue) {
  bool ready = false;
  uint8_t value = 0;
  
  do {
    Serial.print(prompt);
    Serial.print(" (" + String(minValue) + "-" + String(maxValue) + ") [Enter for '" + String(defaultValue) + "']: ");
    String s = readLine();
    value = 0;
    if (s != ""){
      value = atoi(s.c_str());  
    } else {
      value = defaultValue;
    }

    bool valueInvalid = (value == 0 || value < minValue || value > maxValue);
    if (valueInvalid) {
      Serial.println("Error, value outside range.");
    } else {
      //DisplayColor(CHSV( value, 255, 255));     TODO
      Serial.print(value);
      Serial.print("  Confirm? [y/n] ");
      
      while (1) {
        if (Serial.available()) {
          char c = Serial.read();
          if (c == 'y') {
            Serial.println(c);
            preferences.putUChar(key, value);
            ready = true;
            break;
          } else if (c == 'n') {
            Serial.println(c);
            break;
          }
        }
      }
      while (Serial.available()) {
        char c = Serial.read();        
      }
    }
  } while (!ready);

  return value;
}

void Config::PromptAndReadLineDefault(const char* prompt, const char* key, const char* defaultValue) {
  Serial.print(prompt);
  Serial.print(" [Enter for '");
  Serial.print(defaultValue);
  Serial.print("']: ");
  String s = readLine();
  if (s != ""){
    Serial.println(s);
    preferences.putString(key, s);
  } else {
    Serial.println(defaultValue);
    preferences.putString(key, String(defaultValue));
  }
}

String Config::PromptAndReadLine(const char* prompt) {
  Serial.print(prompt);
  String s = readLine();
  Serial.println(s);
  return s;
}

String Config::readLine(void) {
  String line;

  while (1) {
    if (Serial.available()) {
      char c = Serial.read();

      if (c == '\r') {
        // ignore
      } else if (c == '\n') {
        break;
      }

      line += c;
    }
  }

  line.trim();

  return line;
}
