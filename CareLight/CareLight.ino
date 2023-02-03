/**
   BasicHTTPSClient.ino

    Created on: 20.08.2018

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClientSecureBearSSL.h>
// Fingerprint for demo URL, expires on June 2, 2021, needs to be updated well before this date
//const uint8_t fingerprint[20] = { 0x40, 0xaf, 0x00, 0x6b, 0xec, 0x90, 0x22, 0x41, 0x8e, 0xa3, 0xad, 0xfa, 0x1a, 0xe8, 0x25, 0x41, 0x1d, 0x1a, 0x54, 0xb3 };

//#define DEBUG_ESP_HTTP_CLIENT
//#define DEBUG_ESP_PORT Serial

ESP8266WiFiMulti WiFiMulti;
String payload;


void setup() {
  payload.reserve(2500);  
  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("baku", "12837455");
}

String exractParam(String authReq, const String& param, const char delimit) {
  int _begin = authReq.indexOf(param);
  Serial.print(param);
  if (_begin == -1) 
  { 
    Serial.println("Not found");    
    return ""; 
  }
  Serial.println("Found");
  String result = authReq.substring(_begin + param.length(), authReq.indexOf(delimit, _begin + param.length()));
  return result;
}

void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setInsecure();

    HTTPClient https;

    Serial.println(F("Get login page:"));
    if (https.begin(*client, F("https://carelink.minimed.eu/patient/sso/login?country=pl&lang=en"))) {
      https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
      //https.setTimeout(60000);
      https.setReuse(true);
      Serial.print("[HTTPS] GET");
      int httpCode = https.GET();
      
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.print(F("[HTTPS] Response code: ")); 
        Serial.println(httpCode);
        
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          https.getString();
          //Serial.println(payload.c_str());
          
          const String& location = https.getLocation();
          Serial.println(F("Extract sessionID and sessionData:"));
          Serial.println(location.c_str());
          String sessionId = exractParam(location.c_str(), F("sessionID="), '&');
          String sessionData = exractParam(location.c_str(), F("sessionData="), '&');
          Serial.printf("SessionID:   %s\n", sessionId.c_str());
          Serial.printf("SessionData: %s\n\n", sessionData.c_str());       
          https.end();

          Serial.println("Submit login:");
          //if (https.begin(*client, "https://www.wp.pl")) {          
          if (https.begin(*client, F("https://mdtlogin.medtronic.com/mmcl/auth/oauth/v2/authorize/login?locale=en&country=pl"))) {
            https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
            https.setTimeout(60000);
            https.addHeader(F("Accept"), F("text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9"));
            https.addHeader(F("Accept-Language"), F("en-US,en;q=0.9"));
            //https.addHeader("Accept-Encoding", "identity");
            //https.addHeader("sec-ch-ua", "\"Google Chrome\";v=\"87\", \" Not;A Brand\";v=\"99\", \"Chromium\";v=\"87\"");
            https.addHeader(F("Content-Type"), F("application/x-www-form-urlencoded"));
            https.setReuse(true);/////////////////////////////////////
            https.setUserAgent(F("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/109.0.0.0 Safari/537.36 Edg/109.0.1518.70"));                    
            String content = String("sessionID=" + sessionId + "&sessionData=" + sessionData + "&locale=en&action=login&username=sylwiadk&password=Mamba2018&actionButton=Log+in");
            Serial.printf("[HTTPS] POST: %s\n", content.c_str());
            httpCode = https.POST(content);
            Serial.printf("[HTTPS] Response code: %d\n", httpCode);
            if (httpCode > 0) {
              Serial.printf("[HTTPS] Response code: %d\n", httpCode);             

              // file found at server
              if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) 
              {
                payload = https.getString();
                Serial.println(payload.c_str());

                String consentUrl         = exractParam(payload.c_str(), "form action=\"", '"');
                String consentSessionId   = exractParam(payload.c_str(), "\"sessionID\" value=\"", '"');
                String consentSessionData = exractParam(payload.c_str(), "\"sessionData\" value=\"", '"');                
                Serial.printf("consentUrl:         %s\n", consentUrl.c_str());
                Serial.printf("consentSessionData: %s\n", consentSessionData.c_str());
                Serial.printf("ConsentSessionId:   %s\n", consentSessionId.c_str());
                Serial.printf("SessionID:          %s\n", sessionId.c_str());                
                https.end();

                Serial.println("Submit consent:");
                if (https.begin(*client, consentUrl.c_str())) {
                  https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
                  https.setTimeout(60000);
                  https.addHeader(F("Accept"), F("text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9"));
                  https.addHeader(F("Accept-Language"), F("en-US,en;q=0.9"));
                  https.addHeader(F("Content-Type"), F("application/x-www-form-urlencoded"));
                  https.setReuse(true);
                  //https.setUserAgent(F("Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:108.0) Gecko/20100101 Firefox/108.0"));                    
                  content = String("action=consent&sessionID=" + consentSessionId + "&sessionData=" + consentSessionData + "&response_type=code&response_mode=query");
                  Serial.printf("[HTTPS] POST: %s\n", content.c_str());
                  httpCode = https.POST(content.c_str());

                  if (httpCode > 0) {
                    Serial.printf("[HTTPS] Response code: %d\n", httpCode);             

                    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) 
                    {
                      https.getString();
                      //Serial.println(payload.c_str());

                      Serial.println("All cookies:");
                      Serial.println(https.cookies());
                      for(int j=0; j<https.cookies(); j++){
                        Serial.println(https.cookie(j));
                      } 
                      https.end();                      
                    }
                  }
                }                  
              }
            }  
            else
            {
              Serial.printf("[HTTPS] POST failed... failed, error: %s\n", https.errorToString(httpCode).c_str());
            }                               
          }
            
          
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }

  Serial.println("Wait 10s before next round...");
  delay(5000000);
}
