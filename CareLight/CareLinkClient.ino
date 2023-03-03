/////////////////////////////////////////////////////////////////////////////////
// CareLink client functions ///////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

#include <Arduino.h>

#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

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
String currentTrend;
int currentSg;
long long currentServerTime;
long long dataUpdateServerTime; 


void InitCareLinkClient() {
  https.setCookieJar(&cookies);
  client = new WiFiClientSecure();
  client->setInsecure();   
}

bool IsClientConnected() {
  return WiFi.status() == WL_CONNECTED;   
}

void ClearCookies() {
  https.clearAllCookies();
}

bool IsDataStale() {
  return (currentServerTime - dataUpdateServerTime) > 600000;
}

int GetCurrentSg() {
  return currentSg;
}

String GetCurrentTrend() {
  return currentTrend;
}

long long GetUpdateTime() {
  return dataUpdateServerTime;
}

void connectToWiFi() {
  int TryCount = 0;
  while ( WiFi.status() != WL_CONNECTED ) {
    TryCount++;
    WiFi.disconnect();
    WiFi.begin( GetSsid().c_str(), GetPassword().c_str() );
    vTaskDelay( 4000 );
    if ( TryCount == 30 ) {
      ESP.restart();
    }
  }
}

String ExtractParameter(const String& authReq, const String& param, const char delimit) {
  int _begin = authReq.indexOf(param.c_str());
  if (_begin == -1) 
  {    
    return ""; 
  }
  String result = authReq.substring(_begin + param.length(), authReq.indexOf(delimit, _begin + param.length()));
  return result;
}

bool GetLoginPage() {
  //Serial.print("Authentication... ");
  if (https.begin(*client, F("https://carelink.minimed.eu/patient/sso/login?country=pl&lang=en"))) { //TODO country
    https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    https.setTimeout(60000);
    https.setReuse(true);

    httpCode = https.GET();
    //Serial.println(httpCode);
    
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      https.getString();
      const String& location = https.getLocation();
      sessionId = ExtractParameter(location.c_str(), F("sessionID="), '&');
      sessionData = ExtractParameter(location.c_str(), F("sessionData="), '&');      
      https.end();
      return true;
    } else {
      Serial.printf("[HTTPS] GET failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
  https.end();
  return false;
}

bool SubmitLogin () {
  //Serial.println("Submit login:");
  String url = "https://mdtlogin.medtronic.com/mmcl/auth/oauth/v2/authorize/login?locale=en&country=";
  url += GetCareLinkCountry().c_str();
  if (https.begin(*client, url)) {
    https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    https.setTimeout(60000);
    https.addHeader(F("Accept"), F("text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9"));
    https.setReuse(true);
    https.addHeader(F("Content-Type"), F("application/x-www-form-urlencoded"));

    content = String("sessionID=" + sessionId + "&sessionData=" + sessionData + "&locale=en&action=login&username=" + GetCareLinkUser() + "&password=" + GetCareLinkPassword() + "&actionButton=Log+in");    
    //Serial.printf("Content: %s\n", content.c_str());
    
    httpCode = https.POST(content);
    //Serial.println(httpCode);
    
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
    } else {
      Serial.printf("[HTTPS] POST failed, error: %s\n", https.errorToString(httpCode).c_str());
    }                               
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
  https.end();
  return false;
}

bool SubmitConsent() {
  //Serial.println("Submit consent:");
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
    url += GetCareLinkCountry().c_str();
    https.addHeader(F("Referer"), url);
    
    content = String("action=consent&sessionID=" + consentSessionId + "&sessionData=" + consentSessionData + "&response_type=code&response_mode=query");
    //Serial.printf("Content: %s\n", content.c_str());
    httpCode = https.POST(content.c_str());
    //Serial.println(httpCode);

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
    } else {
      Serial.printf("[HTTPS] POST failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
  https.end();
  return false;
}

bool GetData() {
  //Serial.print("Get data: ");
  if (https.begin(*client, "https://clcloud.minimed.eu/connect/v2/display/message")) {
    https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    https.setTimeout(60000);
    https.setUserAgent(F("Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:108.0) Gecko/20100101 Firefox/108.0"));
    https.addHeader(F("Accept"), F("text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8"));
    https.addHeader(F("Accept-Language"), F("en-US,en;q=0.9"));
    https.addHeader(F("Content-Type"), F("application/json; charset=utf-8"));
    https.setReuse(true);
    https.setAuthorizationType("Bearer");
    https.setAuthorization(authToken.c_str());
    content = "{\"username\":\"" + GetCareLinkUser() + "\",\"role\":\"patient\"}";
    httpCode = https.POST(content.c_str());

    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {             
      WiFiClient * stream = https.getStreamPtr();
      StaticJsonDocument<256> filter;
      filter["currentServerTime"] = true;  
      filter["lastMedicalDeviceDataUpdateServerTime"] = true;  
      filter["lastSGTrend"] = true;
      filter["lastSG"]["sg"] = true;
   
      StaticJsonDocument<1024> doc;
      stream->setTimeout(20);
      DeserializationError error = deserializeJson(doc, *stream, DeserializationOption::Filter(filter));

      const char* trend;
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        trend = "";
        currentSg = 0;        
      } else {  
        currentServerTime = doc["currentServerTime"];
        dataUpdateServerTime = doc["lastMedicalDeviceDataUpdateServerTime"];
        trend = doc["lastSGTrend"];
        currentSg = doc["lastSG"]["sg"];   
      }
      currentTrend = String(trend);
      
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

      Serial.print("Trend: ");
      Serial.print(currentTrend);
      Serial.print("  SG: ");
      Serial.print(currentSg);
      Serial.print("  Updated: ");
      long diff = currentServerTime - dataUpdateServerTime;
      Serial.print(diff/60000);
      Serial.print(":");
      Serial.print((diff/1000) - ((diff/60000)*60));
      Serial.print(" ago");      
      Serial.println();

      https.end();
      return true;                
    } else {
      Serial.printf("[HTTPS] POST failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
  https.end();
  return false;
}
