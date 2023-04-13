/*
 Main file of CareLight program 
 Copyright 2023 Jakub Kuchta

 This program is used to drive an indicator LED lamp for people with 
 diabetes using Medtronic 740g/780g insulin pumps. It is based on ESP32 
 programmable module from Espressif and array of 8x8 WS2812B 
 programmable RGB LEDs.

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
#include "Config.h"
#include "Display.h"
#include "CareLinkClient.h"

const unsigned long PollInterval = 61000;
const int OneArrowDifference = 3;
const int TwoArrowDifference = 12;
const int ThreeArrowDifference = 20;

unsigned long previousTime;
unsigned long currentTime;
unsigned long lastSgDatetime;
unsigned long lastSgDatetimeChange;
ArrowType lastArrowType;
int lastSg = 0;
String lastTrend = "";
bool newConnection;
bool isAfterError;

Config config;
Display display(config);
CareLinkClient clClient(config);    

void setup() 
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println(F("\n\nStarting CareLight"));

  display.Init();
  clClient.Init();
  config.Init(display);

  lastSgDatetimeChange = millis();
}

void loop() 
{
  if (!clClient.IsConnected()) 
  {
    display.YellowError();
    display.Update();
    clClient.Connect();
    display.GreenError();
    display.Update();
    previousTime = 0;
    //lastSgDatetime = 0;
    //lastSgDatetimeChange = millis();  //Not sure
    newConnection = true;    
  }  

  currentTime = millis();
  if (currentTime - previousTime >= PollInterval || newConnection) 
  { 
    bool currentDataReceived = GetCareLinkData();
    if (currentDataReceived)
    {
      PrintCareLinkData();

      if (clClient.currentSgDatetime != lastSgDatetime || isAfterError) 
      {
        if (clClient.currentSg == 0 && lastSg == 0)
        {
          // Carelight started while pump does not provide new data
          Serial.println("  Stale data after startup");
          display.BlueError();        
        } 
        else 
        {
          // There is data to check (Sg timestamp changed or recovery after errors)
          Serial.println("  Updating display");
          DisplayCareLinkData(); 
        }       
        
        if (clClient.currentSg != 0) 
        {
          // There is new Sg available, update last values
          lastSgDatetimeChange = currentTime;
          lastSgDatetime = clClient.currentSgDatetime;
          lastSg = clClient.currentSg;
        } 
        else 
        {
          // There is no new reading, only 0 - stale data 
          Serial.println("  Stale data (Current SG = 0)");
        }
        isAfterError = false;
      } 
      else 
      {
        // Nothing new
        Serial.println("  No new readings");
      }
      // Sg timestamp didn't change in last 8 min
      if (currentTime - lastSgDatetimeChange > 600000) 
      {
        Serial.println("  Stale data (>10min)");
        display.BlueError();
      }
    } 
    else if (currentTime - lastSgDatetimeChange > 600000 || newConnection)
    { 
      // Getting data from CareLink failed
      if (clClient.IsConnected()) 
      {
        // And WiFi client is connected - internet connection or CareLink error
        Serial.println("  CareLink API error");
        display.RedError();
      } 
      else 
      {
        // And WiFi client is disconnected - wWiFi error
        Serial.println("  WiFi connection lost");
        display.YellowError();
      }
      isAfterError = true;
    }
    display.Update();
    previousTime = currentTime;
    newConnection = false;
  }
}

bool GetCareLinkData() 
{
  if (!clClient.GetData()) 
  {  
    // If GetData failed  
    if (!clClient.Login()) 
    {      
      // And if Login failed - error
      return false;
    } 
    else 
    {
      // If Login succeded
      if (!clClient.GetData()) 
      { 
        // But GetData still fails - error 
        return false;             
      }
    }
  }
  // GetData succeded
  return true;                    
}

void PrintCareLinkData()
{
  Serial.printf("CurrentTrend: %s  CurrentSg: %d  LastSG: %d  CurrentSgDatetime: %lu  LastSgDatetime: %lu  CurrentTime: %lu  LastSgDatetimeChange: %lu  IsAfterError: %s\n", clClient.currentTrend, clClient.currentSg, lastSg, clClient.currentSgDatetime, lastSgDatetime, currentTime, lastSgDatetimeChange, isAfterError ? "true" : "false");
}

void DisplayCareLinkData() 
{  
  ArrowType currentArrowType = GetCurrentArrowType(clClient.currentSg, lastSg, clClient.currentSgDatetime, lastSgDatetime, clClient.currentTrend);
  uint8_t currentArrowHue = GetCurrentArrowHue(clClient.currentSg, lastSg);
  display.Arrow(currentArrowType, currentArrowHue);
}

uint8_t GetCurrentArrowHue(int sg, int previousSg) {
  if (sg == 0) 
  {
    sg = previousSg;
  }
  if (sg <= config.treshold[0]) {  
    return config.color[0];
  } else if (sg <= config.treshold[1]) {  
    return config.color[1];
  } else if (sg <= config.treshold[2]) {  
    return config.color[2];
  } else if (sg <= config.treshold[3]) {  
    return config.color[3];
  } else if (sg <= config.treshold[4]) {  
    return config.color[4];
  } else if (sg <= config.treshold[5]) {  
    return config.color[5];
  } else if (sg <= config.treshold[6]) {  
    return config.color[6];
  } else {  
    return config.color[7];
  }
}

ArrowType GetCurrentArrowType(int sg, int previousSg, unsigned long sgDatetime, unsigned long previousSgDatetime, const String& sgTrend)
{
  ArrowType currentArrowType = NONE;
  if (sg == 0) 
  {
    Serial.println("  No current SG, using last arrow type");
    currentArrowType = lastArrowType;
  }
  else if (sg <= 40) 
  {
    Serial.println("  Current SG under 40");
    currentArrowType = TOO_LOW;
  } 
  else if (sg >= 400) 
  {
    Serial.println("  Current SG over 400");
    currentArrowType = TOO_HIGH;
  } 
  else if(previousSg == 0) 
  {
    Serial.println("  No last SG, using arrow from current trend");
    currentArrowType = GetArrowTypeFromTrend(sgTrend);
  } 
  else if(sgDatetime <= previousSgDatetime) 
  {
    Serial.println("  Wrong timestamp, using arrow from current trend");
    currentArrowType = GetArrowTypeFromTrend(sgTrend);
  } 
  else 
  {
    float calculatedTrend = CalculateTrend(sg, previousSg, sgDatetime, previousSgDatetime);
    if (calculatedTrend >= ThreeArrowDifference) 
    {
      currentArrowType = UP_TRIPLE;
    } 
    else if (calculatedTrend >= TwoArrowDifference) 
    {
      currentArrowType = UP_DOUBLE;
    } 
    else if (calculatedTrend >= OneArrowDifference) 
    {
      currentArrowType = UP;
    } 
    else if (calculatedTrend > -OneArrowDifference) 
    {
      currentArrowType = STABLE;
    } 
    else if (calculatedTrend > -TwoArrowDifference) 
    {
      currentArrowType = DOWN;
    } 
    else if (calculatedTrend > -ThreeArrowDifference) 
    {
      currentArrowType = DOWN_DOUBLE;
    } 
    else if (calculatedTrend <= -ThreeArrowDifference) 
    {
      currentArrowType = DOWN_TRIPLE;
    } 
    else 
    {
      currentArrowType = NONE;
    }
  }
  lastArrowType = currentArrowType;
  return currentArrowType;
}

ArrowType GetArrowTypeFromTrend(const String& trend) 
{
  String trendString = String(trend);
  if (trendString.equals("UP_TRIPLE")) 
  {
    return UP_TRIPLE;
  } 
  else if (trendString.equals("UP_DOUBLE")) 
  {
    return UP_DOUBLE;
  } 
  else if (trendString.equals("UP")) 
  {
    return UP;
  } 
  else if (trendString.equals("NONE")) 
  {
    return STABLE;
  } 
  else if (trendString.equals("DOWN")) 
  {
    return DOWN;
  } 
  else if (trendString.equals("DOWN_DOUBLE")) 
  {
    return DOWN_DOUBLE;
  } 
  else if (trendString.equals("DOWN_TRIPLE")) 
  {
    return DOWN_TRIPLE;
  } 
  else 
  {
    return NONE;
  } 
}

float CalculateTrend(int sg, int previousSg, unsigned long sgDatetime, unsigned long previousSgDatetime) 
{
  int deltaSg = sg - previousSg;
  Serial.printf("  DeltaSg: %d  ", deltaSg);
  int deltaTimeMinutes = (sgDatetime - previousSgDatetime) / 60;
  Serial.printf("DeltaTime: %d  ", deltaTimeMinutes);
  float slopePer5min = (deltaSg * 5.0) / deltaTimeMinutes;
  Serial.printf("Slope: %f\n", slopePer5min);
  return slopePer5min;
}

