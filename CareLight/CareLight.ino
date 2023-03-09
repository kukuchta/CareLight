#include <Arduino.h>
#include <FastLED.h>

#define POLL_INTERVAL 65000

int oneArrowDifference = 3;
int twoArrowDifference = 12;
int threeArrowDifference = 20;

unsigned long previousMillis;
unsigned long currentMillis;
unsigned long pollInterval;
bool newConnection;

long long lastDataUpdateTime;
String lastTrend = "";
int lastSg = 0;
 

                            

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();

  Serial.println(F("\n\nStarting CareLight"));

  Serial.print("Initializing LED display... ");
  InitDisplay();
  Serial.println("Done");

  Serial.print("Initializing WiFi client... ");
  InitCareLinkClient();
  Serial.println("Done");  
  
  Serial.println("Reading configuration...");
  if (!ArePreferencesSet()){
    Serial.println("Configuration empty, entering setup");
    DefaultPreferences();
    SetDisplayBrightness(GetBrightness());   
    DisplayLogo(); 
    UpdatePreferences();
    Serial.println("Configuration updated");
    ESP.restart();
  } else {
    ReadPreferences();
    PrintPreferences();
    Serial.print("Press any key for setup (in 5 seconds)... ");
    SetDisplayBrightness(GetBrightness());
    DisplayLogo();
    previousMillis = millis();
    while (1) {
      if (Serial.available()) {
        Serial.println("Entering setup");
        UpdatePreferences();
        Serial.println("Configuration updated");
        ESP.restart();
        break;
      }
      currentMillis = millis();
      if (currentMillis - previousMillis >= 5000) {
        Serial.println("Skipped");
        ClearDisplay();        
        break;
      }
    }
  }
}

void loop() {
  if ( !IsClientConnected() ) {
    Serial.println("Initializing WiFi connection started ");
    DisplayYellowError();
    connectToWiFi();
    Serial.println("Initializing WiFi connection done");
    DisplayGreenError();
    previousMillis = 0;
    newConnection = true;
  }  

  currentMillis = millis();
  if (currentMillis - previousMillis >= POLL_INTERVAL || newConnection) {
       
    bool success = PeriodicGetData();
    if (success && IsDataStale()) {
      Serial.println("  Stale data (>10min)");
      DisplayBlueError();
    } else if (success && GetCurrentSg() == 0) {
      Serial.println("  Stale data (Current SG = 0)");
    } else if (success) {
      long long dataUpdateTime = GetUpdateTime();      
      if (dataUpdateTime != lastDataUpdateTime || newConnection) {
        lastDataUpdateTime = dataUpdateTime;
        Serial.println("  Updating display");
        DisplayData();
      } else {
        Serial.println("  No new readings");
      }
    } else if (!success && !IsClientConnected()) {
      Serial.println("  WiFi connection lost");
      DisplayYellowError();
    } else if (!success && IsClientConnected()) {
      Serial.println("  CareLink API error");
      DisplayRedError();
    }
    previousMillis = currentMillis;
    newConnection = false;
  }
}

bool PeriodicGetData() {
  if (!GetData()) {    
    if (!Login()) {       // If GetData failed
      return false;       // And if Login failed - error
    } else {
      if (!GetData()) {   // If Login succeded
        return false;     // But GetData still fails - error
      }
    }
  }
  // GetData succeded
  return true;
}

void DisplayData() {  
  int currentSg = GetCurrentSg();
  String currentTrend = GetCurrentTrend();
  if (currentSg >= 400) {
    Serial.println("  Current SG over 400");
    DisplayTooHighArrow(currentSg);
  } else if (currentSg <= 40) {
    Serial.println("  Current SG under 40");
    DisplayTooLowArrow(currentSg);
  } else if(lastSg == 0) {
    Serial.println("  No last SG, using arrow from current trend");
    DisplayArrowFromTrend(currentTrend, currentSg);
  } else {
    int deltaSg = abs(currentSg - lastSg);
    bool rising = currentSg >= lastSg;
    if (deltaSg >= threeArrowDifference && rising) {
      DisplayTripleUpArrow(currentSg);
    } else if (deltaSg >= twoArrowDifference && deltaSg < threeArrowDifference && rising) {
      DisplayDoubleUpArrow(currentSg);
    } else if (deltaSg >= oneArrowDifference && deltaSg < twoArrowDifference && rising) {
      DisplayUpArrow(currentSg);
    } else if (deltaSg < oneArrowDifference) {
      DisplayStableArrow(currentSg);
    } else if (deltaSg >= oneArrowDifference && deltaSg < twoArrowDifference && !rising) {
      DisplayDownArrow(currentSg);
    } else if (deltaSg >= twoArrowDifference && deltaSg < threeArrowDifference && !rising) {
      DisplayDoubleDownArrow(currentSg);
    } else if (deltaSg >= threeArrowDifference && !rising) {
      DisplayTripleDownArrow(currentSg);
    } else {
      ClearDisplay();
    }
  }
  
  lastSg = currentSg;
}


bool Login() {
  Serial.println("  Authentication started");
  ClearCookies();
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


