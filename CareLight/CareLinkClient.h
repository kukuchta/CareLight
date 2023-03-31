/*
  CareLinkClient.h

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
public:
  int currentSg;
  unsigned long currentSgDatetime;
  String currentTrend;
  CareLinkClient(Config &cfg);
  void Init(void);
  bool IsConnected(void);
  String GetCurrentTrend(void);
  void Connect(void);
  String ExtractParameter(const String& authReq, const String& param, const char delimit);
  bool Login(void);  
  bool GetLoginPage(void);
  bool SubmitLogin (void);
  bool SubmitConsent(void);
  bool GetData(void);
};

#endif