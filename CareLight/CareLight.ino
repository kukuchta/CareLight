#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include <Preferences.h>

#define NUM_LEDS 64
#define DATA_PIN 13
#define POLL_INTERVAL 60000

Preferences preferences;
String ssid = "";  
String password = "";
String careLinkUser = "";
String careLinkPassword = "";
String careLinkCountry = "";
int32_t brightness = 0;
int32_t color[8] = { 0,0,0,0,0,0,0,0};
int32_t treshold[7] = { 0,0,0,0,0,0,0};

unsigned long previousMillis;
unsigned long currentMillis;
unsigned long pollInterval;

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

byte maskLogo[NUM_LEDS] =  {0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,1,0,0,0,0,
                            0,1,1,0,1,0,1,0,
                            0,1,0,0,0,1,0,0,
                            0,1,0,0,1,0,0,0,
                            0,1,1,1,1,0,0,0,
                            0,0,0,0,0,0,0,0,};                            

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();

  Serial.println(F("\n\nStarting CareLight"));

  Serial.println("Configuration...");  
  https.setCookieJar(&cookies);
  
  if (!ArePreferencesSet()){
    Serial.println("Configuration empty, entering setup"); 
    UpdatePreferences();
  } else {
    Serial.print("Press any key for setup (in 5 seconds)... ");
    previousMillis = millis();
    while (1) {
      if (Serial.available()) {
        Serial.println("Entering setup");
        UpdatePreferences();
        break;
      }
      currentMillis = millis();
      if (currentMillis - previousMillis >= 5000) {
        Serial.println("Skipped");
        ReadPreferences();
        break;
      }
    }
  }

  Serial.print("LED init... ");
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness( brightness );
  DisplayFullScale();
  Serial.println("Done");

  pollInterval = POLL_INTERVAL;
  delay(2000);
}

void loop() {
  if (!client) {
    DisplayError(ledYellow);
    Serial.print("Initializing WiFi client... ");
    client = new WiFiClientSecure();
    client->setInsecure();
    Serial.println("Done");    
  }

  if ( !(WiFi.status() == WL_CONNECTED) ) {
    DisplayError(ledYellow);
    Serial.println("Initializing WiFi connection... ");
    connectToWiFi();
    Serial.println("Done");
    previousMillis = millis();
    PeriodicGetData();
  }  

  currentMillis = millis();
  if (currentMillis - previousMillis >= pollInterval) {
    PeriodicGetData();
    previousMillis = currentMillis;
  }
}

bool ArePreferencesSet() {
  preferences.begin("config", false);
  bool result = preferences.isKey("WIFI_SSID");
  preferences.end();
  return result;
}

void UpdatePreferences(){
  preferences.begin("config", false);
  while (Serial.available() > 0) {
    Serial.read();
  }
  String ssid =     PromptAndReadLine("Wifi network name (ssid): ");
  preferences.putString("WIFI_SSID", ssid);  
  String password = PromptAndReadLine("Wifi network password: ");
  preferences.putString("WIFI_PASS", password);
  String careLinkUser = PromptAndReadLine("CareLink user name: ");
  preferences.putString("CARELINK_USER", careLinkUser);
  String careLinkPassword = PromptAndReadLine("CareLink password: ");
  preferences.putString("CARELINK_PASS", careLinkPassword);
  String careLinkCountry = PromptAndReadLineDefault("CareLink country code [Enter for default 'pl']: ", "pl");
  preferences.putString("CARELINK_COUNTRY", careLinkCountry);
  brightness = StoreInt("LED brightness", "BRIGHTNESS", 30, 1, 255);
  
  Serial.println("\nSetup 8 glucose ranges, each with different color.");
  Serial.println("Possible hue values are 1-255, where: \n 1 - Red,\n 12 - Dark orange,\n 32 - Orange,\n 70 - Olive-green,\n 100 - Green,\n 128 - Cyan,\n 164 - Blue,\n 200 - Violet,\n 255 - Red\n");

  Serial.println("Lowest range 1 from 40 mg/dl - set maximum value.");
  treshold[0] = StoreInt("Treshold 1", "TRESHOLD_1", 54, 41, 393);
  Serial.println("Lowest range 1 from 40 to " + String(treshold[0]) + " mg/dl - set color hue.");
  color[0] = StoreInt("Color 1", "COLOR_1", 200, 1, 255);

  Serial.println("Range 2 from " + String(treshold[0] + 1) + " mg/dl - set maximum value.");
  treshold[1] = StoreInt("Treshold 2", "TRESHOLD_2", 69, (treshold[0] + 1), 394);
  Serial.println("Range 2 from " + String(treshold[0] + 1) + " to " + String(treshold[1]) + " mg/dl - set color hue.");
  color[1] = StoreInt("Color 2", "COLOR_2", 164, 1, 255);

  Serial.println("Range 3 from " + String(treshold[1] + 1) + " mg/dl - set maximum value.");
  treshold[2] = StoreInt("Treshold 3", "TRESHOLD_3", 109, (treshold[1] + 1), 395);
  Serial.println("Range 3 from " + String(treshold[1] + 1) + " to " + String(treshold[2]) + " mg/dl - set color hue.");
  color[2] = StoreInt("Color 3", "COLOR_3", 128, 1, 255);

  Serial.println("Range 4 from " + String(treshold[2] + 1) + " mg/dl - set maximum value.");
  treshold[3] = StoreInt("Treshold 4", "TRESHOLD_4", 139, (treshold[2] + 1), 396);
  Serial.println("Range 4 from " + String(treshold[2] + 1) + " to " + String(treshold[3]) + " mg/dl - set color hue.");
  color[3] = StoreInt("Color 4", "COLOR_4", 100, 1, 255);

  Serial.println("Range 5 from " + String(treshold[3] + 1) + " mg/dl - set maximum value.");
  treshold[4] = StoreInt("Treshold 5", "TRESHOLD_5", 179, (treshold[3] + 1), 397);
  Serial.println("Range 5 from " + String(treshold[3] + 1) + " to " + String(treshold[4]) + " mg/dl - set color hue.");
  color[4] = StoreInt("Color 5", "COLOR_5", 70, 1, 255);

  Serial.println("Range 6 from " + String(treshold[4] + 1) + " mg/dl - set maximum value.");
  treshold[5] = StoreInt("Treshold 6", "TRESHOLD_6", 209, (treshold[4] + 1), 398);
  Serial.println("Range 6 from " + String(treshold[4] + 1) + " to " + String(treshold[5]) + " mg/dl - set color hue.");
  color[5] = StoreInt("Color 6", "COLOR_6", 32, 1, 255);

  Serial.println("Range 7 from " + String(treshold[5] + 1) + " mg/dl - set maximum value.");
  treshold[6] = StoreInt("Treshold 7", "TRESHOLD_7", 249, (treshold[5] + 1), 399);
  Serial.println("Range 7 from " + String(treshold[5] + 1) + " to " + String(treshold[6]) + " mg/dl - set color hue.");
  color[6] = StoreInt("Color 7", "COLOR_7", 12, 1, 255);

  Serial.println("Highest range 8 from " + String(treshold[6] + 1) + " to 400 mg/dl - set color hue.");
  color[7] = StoreInt("Color 8", "COLOR_8", 1, 1, 255);
  preferences.end();
}

void ReadPreferences(){
  Serial.println("Configuration:");  
  preferences.begin("config", false);
  ssid = preferences.getString("WIFI_SSID");
  Serial.print("  WiFi: ");
  Serial.println(ssid);  
  password = preferences.getString("WIFI_PASS");
  careLinkUser = preferences.getString("CARELINK_USER");
  Serial.print("  CareLink user: ");
  Serial.println(careLinkUser);
  careLinkPassword = preferences.getString("CARELINK_PASS");
  careLinkCountry = preferences.getString("CARELINK_COUNTRY");
  Serial.print("  CareLink country: ");
  Serial.println(careLinkCountry);
  brightness = preferences.getInt("BRIGHTNESS");
  Serial.print("  LED brightness: ");
  Serial.println(brightness);
  
  treshold[0] = preferences.getInt("TRESHOLD_1");
  treshold[1] = preferences.getInt("TRESHOLD_2");
  treshold[2] = preferences.getInt("TRESHOLD_3");
  treshold[3] = preferences.getInt("TRESHOLD_4");
  treshold[4] = preferences.getInt("TRESHOLD_5");
  treshold[5] = preferences.getInt("TRESHOLD_6");
  treshold[6] = preferences.getInt("TRESHOLD_7");
  for(int i = 0; i<7; i++) {
    Serial.print("  Treshold ");
    Serial.print(i+1);
    Serial.print(": ");    
    Serial.println(treshold[i]);
  }  

  color[0] = preferences.getInt("COLOR_1");
  color[1] = preferences.getInt("COLOR_2");
  color[2] = preferences.getInt("COLOR_3");
  color[3] = preferences.getInt("COLOR_4");
  color[4] = preferences.getInt("COLOR_5");
  color[5] = preferences.getInt("COLOR_6");
  color[6] = preferences.getInt("COLOR_7");
  color[7] = preferences.getInt("COLOR_8");
  for(int i = 0; i<8; i++) {
    Serial.print("  Color ");
    Serial.print(i+1);
    Serial.print(": ");
    Serial.println(color[i]);
  } 

  preferences.end();
}

int32_t StoreInt(const char* prompt, const char* key, int32_t defaultValue, int32_t minValue, int32_t maxValue) {
  bool resultNotOk = false;
  int32_t value = 0;
  
  do {
    Serial.print(prompt);
    Serial.print(" (" + String(minValue) + "-" + String(maxValue) + ") [Enter for default '" + String(defaultValue) + "']: ");
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
      preferences.putInt(key, value);
    }
  } while (resultNotOk);

  return value;
}

String PromptAndReadLineDefault(const char* prompt, const char* defaultValue) {
  Serial.print(prompt);
  String s = readLine();
  if (s != ""){
    Serial.println(s);
    return s;
  } else {
    String def = String(defaultValue);
    Serial.println(def);
    return def;
  }
}

String PromptAndReadLine(const char* prompt) {
  Serial.print(prompt);
  String s = readLine();
  Serial.println(s);
  return s;
}

String readLine() {
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

void connectToWiFi() {
  int TryCount = 0;
  while ( WiFi.status() != WL_CONNECTED ) {
    TryCount++;
    WiFi.disconnect();
    WiFi.begin( ssid.c_str(), password.c_str() );
    vTaskDelay( 4000 );
    if ( TryCount == 10 ) {
      ESP.restart();
    }
  }
}

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

  DisplayArrow(GetArrowFromTrend(currentTrend), GetColorFromSg(currentSg));
  // TODO: comparison with previous values
  lastTrend = currentTrend;
  lastSg = currentSg;
}


/////////////////////////////////////////////////////////////////////////////////
// Display functions ////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

void DisplayRainbow() {
  for (int i=0; i<NUM_LEDS; i++) {
    leds[i] = CHSV( (i/64.0) * 255, 255, 255); 
  }
  FastLED.show();
}

void DisplayFullScale() {
  for (int i=0; i<NUM_LEDS; i+=8) {
      leds[i+0] = CHSV( color[0], 255, 255); 
      leds[i+1] = CHSV( color[1], 255, 255);
      leds[i+2] = CHSV( color[2], 255, 255);
      leds[i+3] = CHSV( color[3], 255, 255);
      leds[i+4] = CHSV( color[4], 255, 255);
      leds[i+5] = CHSV( color[5], 255, 255);
      leds[i+6] = CHSV( color[6], 255, 255);
      leds[i+7] = CHSV( color[7], 255, 255);
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
  if (sg >= 40 && sg <= treshold[0]) {
    color = CHSV( color[0], 255, 255);
  } else if (sg <= treshold[1]) {
    color = CHSV( color[1], 255, 255);
  } else if (sg <= treshold[2]) {
    color = CHSV( color[2], 255, 255);
  } else if (sg <= treshold[3]) {
    color = CHSV( color[3], 255, 255);
  } else if (sg <= treshold[4]) {
    color = CHSV( color[4], 255, 255);
  } else if (sg <= treshold[5]) {
    color = CHSV( color[5], 255, 255);
  } else if (sg <= treshold[6]) {
    color = CHSV( color[6], 255, 255);
  } else if (sg <= 400) {
    color = CHSV( color[7], 255, 255);
  } else {
    color = ledOff;
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


/////////////////////////////////////////////////////////////////////////////////
// CareLink API functions ///////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

String ExtractParameter(const String& authReq, const String& param, const char delimit) {
  int _begin = authReq.indexOf(param.c_str());
  if (_begin == -1) 
  {    
    return ""; 
  }
  String result = authReq.substring(_begin + param.length(), authReq.indexOf(delimit, _begin + param.length()));
  return result;
}

bool Login() {
  Serial.print("Authentication... ");
  https.clearAllCookies();
  bool success = GetLoginPage();

  if (success == true) {
    success = SubmitLogin();
  }

  if (success == true) {
    success = SubmitConsent();
  }

  if (success == true) {
    Serial.println("Done");
  } else {
    Serial.println("Failed");
  }

  return success;
}

bool GetLoginPage() {
  //Serial.print("Authentication... ");
  if (https.begin(*client, F("https://carelink.minimed.eu/patient/sso/login?country=pl&lang=en"))) {
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
  url += careLinkCountry.c_str();
  if (https.begin(*client, url)) {
    https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    https.setTimeout(60000);
    https.addHeader(F("Accept"), F("text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9"));
    https.setReuse(true);
    https.addHeader(F("Content-Type"), F("application/x-www-form-urlencoded"));

    //content = String("sessionID=" + sessionId + "&sessionData=" + sessionData + "&locale=en&action=login&username=sylwiadk&password=Mamba2018&actionButton=Log+in");
    content = String("sessionID=" + sessionId + "&sessionData=" + sessionData + "&locale=en&action=login&username=" + careLinkUser + "&password=" + careLinkPassword + "&actionButton=Log+in");    
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
    url += careLinkCountry.c_str();
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
    content = "{\"username\":\"" + careLinkUser + "\",\"role\":\"patient\"}";
    httpCode = https.POST(content.c_str());

    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {             
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
        currentSg = 0;        
      } else {  
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
      Serial.println(currentSg);

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
