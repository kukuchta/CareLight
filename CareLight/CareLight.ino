#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <FastLED.h>

#define NUM_LEDS 64
#define DATA_PIN 13
#define BRIGHTNESS 60

WiFiClientSecure *client;
HTTPClient https;
String payload;
CookieJar cookies;
String sessionId;
String sessionData;
String authToken;
String consentUrl;
String consentSessionId;
String consentSessionData;
String content;
byte sg;
CRGB leds[NUM_LEDS];
CRGBPalette16 myPal;

int httpCode;

unsigned long previousMillis;
unsigned long currentMillis;
unsigned long period = 300000;

DEFINE_GRADIENT_PALETTE( heatmap_gp ) {
  0,     0,  0,  0,   //<40 black
  1,   122,  1,254,   //40  violet
 23,   255,  0,  0,   //70  red
 24,   255,255,  0,   //71  yellow-green
 51,     1,251,255,   //110 turquoise
 72,     2,222,255,   //140 blue turquoise
100,   255,255,  0,   //180 yellow
150,   255,  0,  0,   //250 red
255,   255,  0,255 }; //400 red

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();
  myPal = heatmap_gp;
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalSMD5050 );
  FastLED.setBrightness( BRIGHTNESS );
  Serial.println("Starting...");
  https.setCookieJar(&cookies);
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

bool GetLoginPage() {
  //Serial.println("Get login page:");
  if (https.begin(*client, F("https://carelink.minimed.eu/patient/sso/login?country=pl&lang=en"))) {
    https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    https.setTimeout(60000);
    https.setReuse(true);

    httpCode = https.GET();
    //Serial.println(httpCode);
    
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      https.getString();
      const String& location = https.getLocation();
      sessionId = exractParam(location.c_str(), F("sessionID="), '&');
      sessionData = exractParam(location.c_str(), F("sessionData="), '&');
      //Serial.printf("SessionID:   %s\n", sessionId.c_str());
      //Serial.printf("SessionData: %s\n\n", sessionData.c_str());       
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

bool SubmitLogin () {
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

bool SubmitConsent() {
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

bool Login() {
  https.clearAllCookies();
  bool success = GetLoginPage();

  if (success == true) {
    success = SubmitLogin();
  }

  if (success == true) {
    success = SubmitConsent();
  }

  return success;
}

bool GetData() {
  Serial.print("Get data: ");
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
    /////Serial.println("Auth cookie:");
    /////Serial.println(authToken.c_str());
    content = "{\"username\":\"sylwiadk\",\"role\":\"patient\"}";
    /////Serial.printf("Content: %s\n", content.c_str());
    httpCode = https.POST(content.c_str());
    /////Serial.println(httpCode);

    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {             
////////////////////////////////////////////////////////////
      WiFiClient * stream = https.getStreamPtr();
      StaticJsonDocument<256> filter;
      filter["lastSGTrend"] = true;
      filter["lastSG"]["sg"] = true;

      StaticJsonDocument<1024> doc;

      DeserializationError error = deserializeJson(doc, *stream, DeserializationOption::Filter(filter));

      const char* lastSGTrend;
      int lastSG_sg;

      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        lastSGTrend = "";
      } else {  
        lastSGTrend = doc["lastSGTrend"]; // "NONE"
        lastSG_sg = doc["lastSG"]["sg"]; // 172        
      }
/////////////////////////////////////////////////////////////////
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
      ////String lastTrend = exractParam(payload, "\"lastSGTrend\":\"", '"');
      ////String lastSg = exractParam(payload, "\"lastSG\":{\"sg\":", ','); 

      Serial.print("Trend: ");
      Serial.print(lastSGTrend);
      Serial.print("  SG: ");
      Serial.print(lastSG_sg);
      sg = byte(lastSG_sg * 0.702777 - 26.111111); // Scale to 1-255
      Serial.print("  Scaled: ");
      Serial.println(sg);
      for (int i=0; i<NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( myPal, sg);
      }
      FastLED.show();
      https.end();/////////////
      return true;                
    }
  }
  https.end();
  return false;
}

void connectToWiFi()
{
  int TryCount = 0;
  while ( WiFi.status() != WL_CONNECTED )
  {
    TryCount++;
    WiFi.disconnect();
    WiFi.begin( "baku2", "12837455" );
    vTaskDelay( 4000 );
    if ( TryCount == 10 )
    {
      ESP.restart();
    }
  }

} // void connectToWiFi()

void PeriodicGetData() {
  bool success = GetData();
  
  if (success != true) {    
    success = Login();
    if (success == true) {
      success = GetData();
    }
  }
}

void loop() {
  if (!client) {
    Serial.println("No WiFi client");
    client = new WiFiClientSecure();
    client->setInsecure();
    Serial.println("WiFi client started");    
  }

  if ( !(WiFi.status() == WL_CONNECTED) ) {
    Serial.println("No WiFi connection");
    connectToWiFi();
    Serial.println("WiFi client connected");
    previousMillis = millis();
    PeriodicGetData();
  }  

  currentMillis = millis();
  if (currentMillis - previousMillis >= period)  //true until the period elapses
  {
    PeriodicGetData();
    previousMillis = currentMillis;
  }
}
