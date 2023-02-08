#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

WiFiMulti WiFiMulti;
WiFiClientSecure *client;
String payload;
CookieJar cookies;
String sessionId;
String sessionData;
String authToken;
String consentUrl;
String consentSessionId;
String consentSessionData;
String content;
int httpCode;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println("Starting...");
  //Serial.flush();
  //delay(3000);
  
  //WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("baku2", "12837455");

  Serial.print("Waiting for WiFi to connect...");
  while ((WiFiMulti.run() != WL_CONNECTED)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" connected");
}

String exractParam(const String& authReq, const String& param, const char delimit) {
  int _begin = authReq.indexOf(param.c_str());
  if (_begin == -1) 
  {    
    return ""; 
  }
  String result = authReq.substring(_begin + param.length(), authReq.indexOf(delimit, _begin + param.length()));
  return result;
}

bool GetLoginPage(WiFiClientSecure *client, HTTPClient& https) {
  Serial.println("Get login page:");
  if (https.begin(*client, F("https://carelink.minimed.eu/patient/sso/login?country=pl&lang=en"))) {
    https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    https.setTimeout(60000);
    https.setReuse(true);

    httpCode = https.GET();
    Serial.println(httpCode);
    
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      https.getString();
      const String& location = https.getLocation();
      sessionId = exractParam(location.c_str(), F("sessionID="), '&');
      sessionData = exractParam(location.c_str(), F("sessionData="), '&');
      Serial.printf("SessionID:   %s\n", sessionId.c_str());
      Serial.printf("SessionData: %s\n\n", sessionData.c_str());       
      https.end();
      return true;
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
  https.end();
  return false;
}

bool SubmitLogin (WiFiClientSecure *client, HTTPClient& https) {
  Serial.println("Submit login:");        
  if (https.begin(*client, F("https://mdtlogin.medtronic.com/mmcl/auth/oauth/v2/authorize/login?locale=en&country=pl"))) {
    https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    https.setTimeout(60000);
    https.addHeader(F("Accept"), F("text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9"));
    https.setReuse(true);
    https.addHeader(F("Content-Type"), F("application/x-www-form-urlencoded"));

    content = String("sessionID=" + sessionId + "&sessionData=" + sessionData + "&locale=en&action=login&username=sylwiadk&password=Mamba2018&actionButton=Log+in");
    Serial.printf("Content: %s\n", content.c_str());
    
    httpCode = https.POST(content);
    Serial.println(httpCode);
    
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        payload = https.getString();
        //Serial.println(payload.c_str());

        consentUrl         = exractParam(payload, "form action=\"", '"');
        consentSessionId   = exractParam(payload, "\"sessionID\" value=\"", '"');
        consentSessionData = exractParam(payload, "\"sessionData\" value=\"", '"');                
        Serial.printf("ConsentUrl:         %s\n", consentUrl.c_str());
        Serial.printf("ConsentSessionData: %s\n", consentSessionData.c_str());
        Serial.printf("ConsentSessionId:   %s\n\n", consentSessionId.c_str());

        https.end();
        return true;                  
    }  
    else
    {
      Serial.printf("[HTTPS] POST failed... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }                               
  }
  https.end();
  return false;
}

bool SubmitConsent(WiFiClientSecure *client, HTTPClient& https) {
  Serial.println("Submit consent:");
  if (https.begin(*client, consentUrl.c_str())) {
    https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    https.setTimeout(60000);

    https.setUserAgent(F("Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:108.0) Gecko/20100101 Firefox/108.0"));
    https.addHeader(F("Accept"), F("text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8"));
    https.addHeader(F("Accept-Language"), F("en-US,en;q=0.9"));
    https.addHeader(F("Content-Type"), F("application/x-www-form-urlencoded"));
    https.addHeader(F("Origin"), F("https://mdtlogin.medtronic.com"));
    https.setReuse(true);
    https.addHeader(F("Referer"), F("https://mdtlogin.medtronic.com/mmcl/auth/oauth/v2/authorize/login?locale=en&country=pl"));
    
    content = String("action=consent&sessionID=" + consentSessionId + "&sessionData=" + consentSessionData + "&response_type=code&response_mode=query");
    Serial.printf("Content: %s\n", content.c_str());
    httpCode = https.POST(content.c_str());
    Serial.println(httpCode);

    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {             
      https.getString();
      for (int i = 0; i < cookies.size(); i++){
        if(cookies[i].name.equals("auth_tmp_token")){
          authToken = cookies[i].value;
          Serial.println("Auth cookie:");
          Serial.println(authToken.c_str());
        }
      }
      https.end();
      return true;
    }
  }
  https.end();
  return false;
}

bool Login(WiFiClientSecure *client, HTTPClient& https) {
  bool success = GetLoginPage(client, https);

  if (success == true) {
    success = SubmitLogin(client, https);
  }

  if (success == true) {
    success = SubmitConsent(client, https);
  }

  return success;
}

String GetData(WiFiClientSecure *client, HTTPClient& https) {
  Serial.println("Get data:");
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
    Serial.println("Auth cookie:");
    Serial.println(authToken.c_str());
    content = "{\"username\":\"sylwiadk\",\"role\":\"patient\"}";
    Serial.printf("Content: %s\n", content.c_str());
    httpCode = https.POST(content.c_str());
    Serial.println(httpCode);

    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {             
      payload = https.getString();//                    "lastSGTrend":"
      String lastTrend = exractParam(payload, "\"lastSGTrend\":\"", '"');
      String lastSg = exractParam(payload, "\"lastSG\":{\"sg\":", ','); 

      Serial.print("Trend: ");
      Serial.println(lastTrend.c_str());
      Serial.print("SG: ");
      Serial.println(lastSg.c_str());
      https.end();
      return lastSg;                
    }
  }
  https.end();
  return "";
}

void loop() {
  if (!client) {
    client = new WiFiClientSecure();
    client->setInsecure();
  }
  
  {
    HTTPClient https;
    https.setCookieJar(&cookies);
    
    bool success = Login(client, https);

    if (success == true) {
      GetData(client, https);
    }
  }

  Serial.println("Wait 10s before next round...");
  delay(5000000);
}
