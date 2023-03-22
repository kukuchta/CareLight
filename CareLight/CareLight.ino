#include <Arduino.h>
#include <FastLED.h>

#define POLL_INTERVAL 61000

int oneArrowDifference = 3;
int twoArrowDifference = 12;
int threeArrowDifference = 20;

unsigned long previousTime;
unsigned long currentTime;
unsigned long pollInterval;
unsigned long lastSgDatetimeChange;
bool newConnection;
bool isAfterError;

String lastTrend = "";
int lastSg = 0;
unsigned long lastSgDatetime = 0;

                            

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
    previousTime = millis();
    while (1) {
      if (Serial.available()) {
        Serial.println("Entering setup");
        UpdatePreferences();
        Serial.println("Configuration updated");
        ESP.restart();
        break;
      }
      currentTime = millis();
      if (currentTime - previousTime >= 5000) {
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
    previousTime = 0;
    lastSgDatetime = 0;
    lastSgDatetimeChange = millis();
    newConnection = true;    
  }  

  currentTime = millis();
  if (currentTime - previousTime >= POLL_INTERVAL || newConnection) { 
    bool currentDataReceived = PeriodicGetData();

    if (currentDataReceived) {
      Serial.printf("Trend: %s  SG: %d  Updated: %lu\n", GetCurrentTrend(), GetCurrentSg(), GetCurrentSgDatetime());
      if (currentTime - lastSgDatetimeChange > 600000) {
        Serial.println("  Stale data (>10min)");
        DisplayBlueError();
      } else {
        if (GetCurrentSgDatetime() != lastSgDatetime || isAfterError) {
          if (GetCurrentSg() == 0) {
            Serial.println("  Stale data (Current SG = 0)"); //TODO: Filter out single readings with 0
            DisplayBlueError();
          } else {
            Serial.println("  Updating display");
            DisplayData();
            lastSgDatetimeChange = currentTime;
            lastSgDatetime = GetCurrentSgDatetime();
            lastSg = GetCurrentSg();
          }
          isAfterError = false;
        } else {
          Serial.println("  No new readings");
        }
      }
    } else {
      if (IsClientConnected()) {
        Serial.println("  CareLink API error");
        DisplayRedError();
      } else {
        Serial.println("  WiFi connection lost");
        DisplayYellowError();
      }
      isAfterError = true;
    }
    
    previousTime = currentTime;
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
  if (currentSg >= 400) {
    Serial.println("  Current SG over 400");
    DisplayTooHighArrow(currentSg);
  } else if (currentSg <= 40) {
    Serial.println("  Current SG under 40");
    DisplayTooLowArrow(currentSg);
  } else if(lastSg == 0) {
    Serial.println("  No last SG, using arrow from current trend");
    DisplayArrowFromTrend(GetCurrentTrend(), currentSg);
  } else if(GetCurrentSgDatetime() <= lastSgDatetime) {
    Serial.println("  Wrong timestamp, using arrow from current trend");
    DisplayArrowFromTrend(GetCurrentTrend(), currentSg);
  } else if(isAfterError) {  // TODO: Handle differently to not share this global flag
    Serial.println("  Recovering after error, using arrow from current trend");
    DisplayArrowFromTrend(GetCurrentTrend(), currentSg);
  } else {
    int deltaSg = abs(currentSg - lastSg);
    Serial.printf("  DeltaSg: %d  ", deltaSg);
    int deltaTimeMinutes = (GetCurrentSgDatetime() - lastSgDatetime) / 60;
    Serial.printf("DeltaTime: %d  ", deltaTimeMinutes);
    float slopePer5min = (deltaSg * 5) / deltaTimeMinutes;
    Serial.printf("Slope: %f\n", slopePer5min);
    bool rising = currentSg >= lastSg;
    if (slopePer5min >= threeArrowDifference && rising) {
      DisplayTripleUpArrow(currentSg);
    } else if (slopePer5min >= twoArrowDifference && slopePer5min < threeArrowDifference && rising) {
      DisplayDoubleUpArrow(currentSg);
    } else if (slopePer5min >= oneArrowDifference && slopePer5min < twoArrowDifference && rising) {
      DisplayUpArrow(currentSg);
    } else if (slopePer5min < oneArrowDifference) {
      DisplayStableArrow(currentSg);
    } else if (slopePer5min >= oneArrowDifference && slopePer5min < twoArrowDifference && !rising) {
      DisplayDownArrow(currentSg);
    } else if (slopePer5min >= twoArrowDifference && slopePer5min < threeArrowDifference && !rising) {
      DisplayDoubleDownArrow(currentSg);
    } else if (slopePer5min >= threeArrowDifference && !rising) {
      DisplayTripleDownArrow(currentSg);
    } else {
      ClearDisplay();
    }
  }
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


