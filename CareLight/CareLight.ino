#include <Arduino.h>
#include <FastLED.h>



#define POLL_INTERVAL 60000

int oneArrowDifference = 3;
int twoArrowDifference = 12;
int threeArrowDifference = 20;

unsigned long previousMillis;
unsigned long currentMillis;
unsigned long pollInterval;
bool newConnection;


long long lastDataUpdateServerTime;
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
    DisplayRainbow(); 
    UpdatePreferences();
    Serial.println("Configuration updated");
  } else {
    ReadPreferences();
    PrintPreferences();
    Serial.print("Press any key for setup (in 5 seconds)... ");
    SetDisplayBrightness(GetBrightness());
    
    DisplayFullScale();
    previousMillis = millis();
    while (1) {
      if (Serial.available()) {
        Serial.println("Entering setup");
        UpdatePreferences();
        Serial.println("Configuration updated");
        break;
      }
      currentMillis = millis();
      if (currentMillis - previousMillis >= 5000) {
        Serial.println("Skipped");
        break;
      }
    }
  }
}

void loop() {
  if ( !IsClientConnected() ) {
    DisplayYellowError();
    Serial.print("Initializing WiFi connection... ");
    connectToWiFi();
    Serial.println("Done");
    previousMillis = 0;
    newConnection = true;
  }  

  currentMillis = millis();
  if (currentMillis - previousMillis >= POLL_INTERVAL || newConnection) {
    newConnection = false;   
    PeriodicGetData();
    previousMillis = currentMillis;
  }
}

void PeriodicGetData() {
  bool success = GetData();  // Try get data
  
  if (success != true) {    
    success = Login();       // If failed - do Login
    if (success != true && IsClientConnected()) {
      DisplayRedError();  // If login failed - error
      return;
    } else {
      success = GetData();   // Retry get data
      if (success != true && IsClientConnected()) {
        DisplayRedError();// If failed again - error
        return;
      }
    }
  }

  int currentSg = GetCurrentSg();
  String currentTrend = GetCurrentTrend();
  long long dataUpdateTime = GetUpdateTime();
  if (IsDataStale()) {
    DisplayBlueError();
    Serial.println("  Stale data (>10min)");
  } else if (currentSg == 0) {
    DisplayBlueError();
    Serial.println("  Stale data (Current SG = 0)");
  } else if (GetUpdateTime() != lastDataUpdateServerTime) {
    Serial.println("  Updating arrow and last SG/Update time");
    lastDataUpdateServerTime = GetUpdateTime();
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
  } else {
    Serial.println("  No new readings");
  }
}

bool Login() {
  Serial.print("Authentication... ");
  ClearCookies();
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


