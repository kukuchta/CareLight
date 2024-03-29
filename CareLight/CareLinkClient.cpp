/*
 CareLinkClient.cpp is a part of CareLight program
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
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <StreamUtils.h>
#include <time.h>
#include "CareLinkClient.h"
#include "Config.h"

CareLinkClient::CareLinkClient(Config &cfg) : config(cfg) 
{ }

void CareLinkClient::Init(void)
{
  Serial.print("Initializing CareLink client... ");
  https.setCookieJar(&cookies);
  client = new WiFiClientSecure();
  client->setInsecure(); 
  Serial.println("Done"); 
}

bool CareLinkClient::IsConnected(void) {
  return WiFi.status() == WL_CONNECTED;   
}

void CareLinkClient::Connect(void) {
  Serial.println("Initializing WiFi connection started ");
  int TryCount = 0;
  while ( WiFi.status() != WL_CONNECTED ) {
    TryCount++;
    WiFi.disconnect();
    WiFi.begin( config.ssid.c_str(), config.password.c_str() );
    vTaskDelay( 5000 );
    if ( TryCount == 60 ) {
      ESP.restart();
    }
  }
  Serial.println("Initializing WiFi connection done");
}

String CareLinkClient::ExtractParameter(const String& authReq, const String& param, const char delimit) {
  int _begin = authReq.indexOf(param.c_str());
  if (_begin == -1) 
  {    
    return ""; 
  }
  String result = authReq.substring(_begin + param.length(), authReq.indexOf(delimit, _begin + param.length()));
  return result;
}

bool CareLinkClient::Login() {
  Serial.println("  Authentication started");
  https.clearAllCookies();
  bool success = GetLoginPage();

  if (success == true) {
    success = SubmitLogin();
  }

  if (success == true) {
    success = SubmitConsent();
  }

  if (success == true) {
    Serial.println("  Authentication done");
  } else {
    Serial.println("  Authentication failed");
  }

  return success;
}

bool CareLinkClient::GetLoginPage(void) {
  Serial.print("  Get login page: ");
  if (https.begin(*client, F("https://carelink.minimed.eu/patient/sso/login?country=pl&lang=en"))) { //TODO country
    https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    https.setTimeout(60000);
    https.setReuse(true);

    httpCode = https.GET();
    Serial.println(httpCode);
    
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      https.getString();
      const String& location = https.getLocation();
      sessionId = ExtractParameter(location.c_str(), F("sessionID="), '&');
      sessionData = ExtractParameter(location.c_str(), F("sessionData="), '&');      
      https.end();
      return true;
    } else if (httpCode < 0) {
      Serial.printf("  Get login page failed, %s\n", https.errorToString(httpCode).c_str());
    } else {
      Serial.println("  Get login page failed");
    }
  } else {
    Serial.println("  Get login page failed, unable to connect");
  }
  https.end();
  return false;
}

bool CareLinkClient::SubmitLogin(void) {
  Serial.print("  Submit login: ");
  String url = "https://mdtlogin-ocl.medtronic.com/mmcl/auth/oauth/v2/authorize/login?locale=en&country=";
  url += config.careLinkCountry.c_str();
  if (https.begin(*client, url)) {
    https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    https.setTimeout(60000);
    https.addHeader(F("Accept"), F("text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9"));
    https.setReuse(true);
    https.addHeader(F("Content-Type"), F("application/x-www-form-urlencoded"));

    content = String("sessionID=" + sessionId + "&sessionData=" + sessionData + "&locale=en&action=login&username=" + config.careLinkUser + "&password=" + config.careLinkPassword + "&actionButton=Log+in");    
    //Serial.printf("Content: %s\n", content.c_str());
    
    httpCode = https.POST(content);
    Serial.println(httpCode);
    
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        payload = https.getString();
        //Serial.println(payload.c_str());

        consentUrl         = ExtractParameter(payload, "form action=\"", '"');
        consentSessionId   = ExtractParameter(payload, "\"sessionID\" value=\"", '"');
        consentSessionData = ExtractParameter(payload, "\"sessionData\" value=\"", '"');                
        //Serial.printf("ConsentUrl:         %s\n", consentUrl.c_str());
        //Serial.printf("ConsentSessionData: %s\n", consentSessionData.c_str());
        //Serial.printf("ConsentSessionId:   %s\n\n", consentSessionId.c_str());

        https.end();
        return true;                  
    } else if (httpCode < 0) {
      Serial.printf("  Submit login failed, %s\n", https.errorToString(httpCode).c_str());
    } else {
      Serial.println("  Submit login failed");
    }                               
  } else {
    Serial.println("  Submit login failed, unable to connect");
  }
  https.end();
  return false;
}

bool CareLinkClient::SubmitConsent(void) {
  Serial.print("  Submit consent: ");
  if (https.begin(*client, consentUrl.c_str())) {
    https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    https.setTimeout(60000);

    https.setUserAgent(F("Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:108.0) Gecko/20100101 Firefox/108.0"));
    https.addHeader(F("Accept"), F("text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8"));
    https.addHeader(F("Accept-Language"), F("en-US,en;q=0.9"));
    https.addHeader(F("Content-Type"), F("application/x-www-form-urlencoded"));
    https.addHeader(F("Origin"), F("https://mdtlogin.medtronic.com"));
    https.setReuse(true);
    String url = "https://mdtlogin.medtronic.com/mmcl/auth/oauth/v2/authorize/login?locale=en&country=";
    url += config.careLinkCountry.c_str();
    https.addHeader(F("Referer"), url);
    
    content = String("action=consent&sessionID=" + consentSessionId + "&sessionData=" + consentSessionData + "&response_type=code&response_mode=query");
    //Serial.printf("Content: %s\n", content.c_str());
    httpCode = https.POST(content.c_str());
    Serial.println(httpCode);

    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {             
      https.getString();
      for (int i = 0; i < cookies.size(); i++){
        if(cookies[i].name.equals("auth_tmp_token")){
          authToken = cookies[i].value;
          //Serial.println("Auth cookie:");
          //Serial.println(authToken.c_str());
          //Serial.println();
          //Serial.println();
        }
      }
      https.end();
      return true;
    } else if (httpCode < 0) {
      Serial.printf("  Submit consent failed, %s\n", https.errorToString(httpCode).c_str());
    } else {
      Serial.println("  Submit consent failed");
    }
  } else {
    Serial.println("  Submit consent failed, unable to connect");
  }
  https.end();
  return false;
}

bool CareLinkClient::GetData(void) {
  Serial.print("  Get data: ");
  if (https.begin(*client, "https://clcloud.minimed.eu/connect/v2/display/message")) {
    https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    https.setTimeout(30000);
    https.setUserAgent(F("Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:108.0) Gecko/20100101 Firefox/108.0"));
    https.addHeader(F("Accept"), F("text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8"));
    https.addHeader(F("Accept-Language"), F("en-US,en;q=0.9"));
    https.addHeader(F("Content-Type"), F("application/json; charset=utf-8"));
    https.setReuse(true);
    https.setAuthorizationType("Bearer");
    https.setAuthorization(authToken.c_str());
    content = "{\"username\":\"" + config.careLinkUser + "\",\"role\":\"patient\"}";
    httpCode = https.POST(content.c_str());
    Serial.println(httpCode);

    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {             
      WiFiClient * stream = https.getStreamPtr();
      StaticJsonDocument<512> filter;
      filter["lastSGTrend"] = true;
      filter["lastSG"]["sg"] = true;
      filter["lastSG"]["datetime"] = true;
   
      StaticJsonDocument<1024> doc;
      stream->setTimeout(100);
      DeserializationError error = deserializeJson(doc, *stream, DeserializationOption::Filter(filter));

      //ReadLoggingStream loggingStream(*stream, Serial);
      //loggingStream.setTimeout(100);      
      //Serial.print("    JSON start\n    ");      
      //DeserializationError error = deserializeJson(doc, loggingStream, DeserializationOption::Filter(filter));
      //Serial.println("\n    JSON end");
      const char* trend;
      const char* datetime;
      if (error) {
        Serial.print("  Data deserialization failed, ");
        Serial.println(error.c_str());
        return false;
      } else {  
        trend = doc["lastSGTrend"];
        currentSg = doc["lastSG"]["sg"];
        datetime = doc["lastSG"]["datetime"];   
      }
      currentTrend = String(trend);
      struct tm t;
      memset(&t, 0, sizeof(t));
      strptime(datetime, "%Y-%m-%dT%H:%M:%S", &t);
      currentSgDatetime = mktime(&t);
      
      // Print payload //////////////////////////////////////////////////
      //Serial.print("[");
      //int len = https.getSize();
      //uint8_t buff[512];
      //// get tcp stream
      //WiFiClient * stream = https.getStreamPtr();
      //// read all data from server
      //while(https.connected() && (len > 0 || len == -1)) {
      //    // get available data size
      //    size_t size = stream->available();
      //    if(size) {
      //        // read up to 128 byte
      //        int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
      //        // write it to Serial
      //        Serial.write(buff, c);
      //        if(len > 0) {
      //            len -= c;
      //        }
      //    }
      //    delay(1);
      //}   
      //Serial.println("] End");       
      ////////////////////////////////////////////////////////////////////

      https.end();
      return true;                
    } else if (httpCode < 0) {
      Serial.printf("  Get data failed, %s\n", https.errorToString(httpCode).c_str());
    } else {
      Serial.println("  Get data failed");
    }
  } else {
    Serial.println("  Get data failed, unable to connect");
  }
  https.end();
  return false;
}