#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <FastLED.h>

#define NUM_LEDS 64
#define DATA_PIN 13
#define BRIGHTNESS 30
#define POLL_INTERVAL 300000

unsigned long previousMillis;
unsigned long currentMillis;

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
String lastTrend;
int lastSg;

CRGB leds[NUM_LEDS];
CRGB ledOff = CHSV( 0, 0, 0);
CRGB ledRed = CHSV( 0, 255, 255);
CRGB ledYellow = CHSV( 58, 255, 255);
CRGB ledBlue = CHSV( 164, 255, 255);
CRGB ledColors[8] = {
  CHSV( 200, 255, 255), //violet
  CHSV( 164, 255, 255), //blue
  CHSV( 128, 255, 255), //cyan
  CHSV( 100, 255, 255), //green
  CHSV( 70,  255, 255), //olive-green
  CHSV( 32,  255, 255), //orange
  CHSV( 12,  255, 255), //dark orange
  CHSV( 0,   255, 255)  //red
};

byte maskUp[NUM_LEDS] = {1,1,1,1,1,1,1,1,
                         1,1,1,1,1,1,1,1,
                         0,0,0,0,0,0,1,1,
                         0,0,0,0,0,0,1,1,
                         0,0,0,0,0,0,1,1,
                         0,0,0,0,0,0,1,1,
                         0,0,0,0,0,0,1,1,
                         0,0,0,0,0,0,1,1,};

byte maskDoubleUp[NUM_LEDS] = {1,1,1,1,1,1,1,1,
                               1,1,1,1,1,1,1,1,
                               0,0,0,0,0,0,1,1,
                               1,1,1,1,1,0,1,1,
                               1,1,1,1,1,0,1,1,
                               0,0,0,1,1,0,1,1,
                               0,0,0,1,1,0,1,1,
                               0,0,0,1,1,0,1,1,};

byte maskTripleUp[NUM_LEDS] = {1,1,1,1,1,1,1,1,
                               1,1,1,1,1,1,1,1,
                               0,0,0,0,0,0,1,1,
                               1,1,1,1,1,0,1,1,
                               1,1,1,1,1,0,1,1,
                               0,0,0,1,1,0,1,1,
                               1,1,0,1,1,0,1,1,
                               1,1,0,1,1,0,1,1,};

byte maskStable[NUM_LEDS] = {0,0,0,0,0,0,1,1,
                             0,0,0,0,0,0,1,1,
                             0,0,0,0,0,0,1,1,
                             0,0,0,0,0,0,1,1,
                             0,0,0,0,0,0,1,1,
                             0,0,0,0,0,0,1,1,
                             1,1,1,1,1,1,1,1,
                             1,1,1,1,1,1,1,1,};

byte maskDown[NUM_LEDS] = {1,1,0,0,0,0,0,0,
                           1,1,0,0,0,0,0,0,
                           1,1,0,0,0,0,0,0,
                           1,1,0,0,0,0,0,0,
                           1,1,0,0,0,0,0,0,
                           1,1,0,0,0,0,0,0,
                           1,1,1,1,1,1,1,1,
                           1,1,1,1,1,1,1,1,};

byte maskDoubleDown[NUM_LEDS] = {1,1,0,1,1,0,0,0,
                                 1,1,0,1,1,0,0,0,
                                 1,1,0,1,1,0,0,0,
                                 1,1,0,1,1,1,1,1,
                                 1,1,0,1,1,1,1,1,
                                 1,1,0,0,0,0,0,0,
                                 1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,};

byte maskTripleDown[NUM_LEDS] = {1,1,0,1,1,0,1,1,
                                 1,1,0,1,1,0,1,1,
                                 1,1,0,1,1,0,0,0,
                                 1,1,0,1,1,1,1,1,
                                 1,1,0,1,1,1,1,1,
                                 1,1,0,0,0,0,0,0,
                                 1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,};

byte maskError[NUM_LEDS] = {0,0,0,0,0,0,1,1,
                            0,0,0,0,0,1,1,1,
                            0,0,0,0,1,1,1,0,
                            0,0,0,1,1,1,0,0,
                            0,0,0,1,1,0,0,0,
                            0,0,0,0,0,0,0,0,
                            1,1,0,0,0,0,0,0,
                            1,1,0,0,0,0,0,0,};

byte maskLow[NUM_LEDS] = {1,0,0,0,0,0,1,1,
                          1,0,0,0,0,1,1,1,
                          1,0,0,0,1,1,1,0,
                          1,0,0,0,1,1,0,0,
                          1,0,1,1,0,0,0,0,
                          1,0,1,1,0,0,0,0,
                          1,0,0,0,0,0,0,0,
                          1,1,1,1,1,1,1,1,};

byte maskHi[NUM_LEDS] = {1,1,1,1,1,1,1,1,
                         0,0,0,0,0,0,0,1,
                         0,0,0,0,1,1,0,1,
                         0,0,0,1,1,1,0,1,
                         0,0,1,1,1,0,0,1,
                         0,0,1,1,0,0,0,1,
                         1,1,0,0,0,0,0,1,
                         1,1,0,0,0,0,0,1,};

byte maskEmpty[NUM_LEDS] = {0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,};

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness( BRIGHTNESS );
  Serial.println("Starting...");
  https.setCookieJar(&cookies);
  DisplayFullScale();
}

void DisplayRainbow() {
  for (int i=0; i<NUM_LEDS; i++) {
    leds[i] = CHSV( (i/64.0) * 255, 255, 255); 
  }
  FastLED.show();
}

void DisplayFullScale() {
  for (int i=0; i<NUM_LEDS; i+=8) {
    for (int j=0; j<8; j++) {
        leds[i+j] = ledColors[j]; 
    }
  }
  FastLED.show();
}

void DisplayError(CRGB color) {
  for (int i=0; i<NUM_LEDS; i++) {
    if (maskError[i] == 1) {
      leds[i] = color; 
    } else {
      leds[i] = ledOff;
    }
  }
  FastLED.show();
}

void DisplayArrow(byte* mask, CRGB color) {
  for (int i=0; i<NUM_LEDS; i++) {
    if (mask[i] == 1) {
      leds[i] = color;
    } else {
      leds[i] = ledOff;
    }
  }
  FastLED.show();
}

void DisplayHi (CRGB color) {
  for (int i=0; i<NUM_LEDS; i++) {
    if (maskHi[i] == 1) {
      leds[i] = color;
    } else {
      leds[i] = ledOff;
    }
  }
  FastLED.show();
}

void DisplayLow(CRGB color) {
  for (int i=0; i<NUM_LEDS; i++) {
    if (maskLow[i] == 1){
      leds[i] = color;
    } else {
      leds[i] = ledOff;
    }
  }
  FastLED.show();
}

CRGB GetColorFromSg(int sg) {
  CRGB color;
  switch (sg) {
    case 40 ... 54:
      color = ledColors[0];
      break;
    case 55 ... 69:
      color = ledColors[1];
      break;
    case 70 ... 99:
      color = ledColors[2];
      break;
    case 100 ... 139:
      color = ledColors[3];
      break;
    case 140 ... 179:
      color = ledColors[4];
      break;
    case 180 ... 209:
      color = ledColors[5];
      break;
    case 210 ... 249:
      color = ledColors[6];
      break;
    case 250 ... 400:
      color = ledColors[7];
      break;
    default: 
      color = ledOff;
      break;
  }
  return color;
}

byte* GetArrowFromTrend(const String& trend) {
  String trendString = String(trend);
  if (trendString.equals("UP_TRIPLE")) {
    return maskTripleUp;
  } else if (trendString.equals("UP_DOUBLE")) {
    return maskDoubleUp;
  } else if (trendString.equals("UP")) {
    return maskUp;
  } else if (trendString.equals("NONE")) {
    return maskStable;
  } else if (trendString.equals("DOWN")) {
    return maskDown;
  } else if (trendString.equals("DOWN_DOUBLE")) {
    return maskDoubleDown;
  } else if (trendString.equals("DOWN_TRIPLE")) {
    return maskTripleDown;
  } else {
    return maskEmpty;
  } 
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
          Serial.println();
          Serial.println();
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

      const char* trend;
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        trend = "";
        lastSg = 0;        
      } else {  
        trend = doc["lastSGTrend"]; // "NONE"
        lastSg = doc["lastSG"]["sg"]; // 172        
      }
      lastTrend = String(trend);
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

      Serial.print("Trend: ");
      Serial.print(lastTrend);
      Serial.print("  SG: ");
      Serial.println(lastSg);

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
  bool success = GetData();  // Try get data
  
  if (success != true) {    
    success = Login();       // If failed - do Login
    if (success != true) {
      DisplayError(ledRed);  // If login failed - error
      return;
    } else {
      success = GetData();   // Retry get data
      if (success != true) {
        DisplayError(ledRed);// If failed again - error
        return;
      }
    }
  }

  DisplayArrow(GetArrowFromTrend(lastTrend), GetColorFromSg(lastSg));
  
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
  if (currentMillis - previousMillis >= POLL_INTERVAL) {
    PeriodicGetData();
    previousMillis = currentMillis;
  }
}
