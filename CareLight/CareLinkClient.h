/*
 CareLinkClient.h is a part of CareLight program
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

#ifndef CARELINKCLIENT_h
#define CARELINKCLIENT_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <StreamUtils.h>
#include <time.h>
#include "Config.h"

class CareLinkClient
{
public:
  int currentSg;
  unsigned long currentSgDatetime;
  String currentTrend;
  CareLinkClient(Config &cfg);
  void Init(void);
  bool IsConnected(void);
  void Connect(void);
  bool Login(void);  
  bool GetData(void);
private:
  Config &config;
  WiFiClientSecure *client;
  HTTPClient https;
  int httpCode;
  String payload;
  CookieJar cookies;
  String sessionId;
  String sessionData;
  String authToken;
  String consentUrl;
  String consentSessionId;
  String consentSessionData;
  String content;
  String ExtractParameter(const String& authReq, const String& param, const char delimit);
  bool GetLoginPage(void);
  bool SubmitLogin (void);
  bool SubmitConsent(void);
};

#endif