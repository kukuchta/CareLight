#include <Arduino.h>
#include <FastLED.h>
#include "Config.h"
#include "Display.h"
#include "CareLinkClient.h"

const unsigned long PollInterval = 61000;
const int OneArrowDifference = 3;
const int TwoArrowDifference = 12;
const int ThreeArrowDifference = 20;

unsigned long previousTime;
unsigned long currentTime;
unsigned long lastSgDatetime;
unsigned long lastSgDatetimeChange;
int lastSg = 0;
String lastTrend = "";
bool newConnection;
bool isAfterError;

Config config;
Display display(config);
CareLinkClient clClient(config);    

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println(F("\n\nStarting CareLight"));

  display.Init();
  clClient.Init();
  PrepareConfig();
}

void loop() {
  if ( !clClient.IsConnected() ) {
    display.YellowError();
    clClient.Connect();
    display.GreenError();
    previousTime = 0;
    lastSgDatetime = 0;
    lastSgDatetimeChange = millis();
    newConnection = true;    
  }  

  currentTime = millis();
  if (currentTime - previousTime >= PollInterval || newConnection) { 
    bool currentDataReceived = PeriodicGetData();

    if (currentDataReceived) {
      PrintData();
      
      // mamy dane do sprawdzenia (przyszedł inny timestamp albo recovery po błędzie)
      if (clClient.currentSgDatetime != lastSgDatetime || isAfterError) {
        if (clClient.currentSg != 0) {
          // mamy nowy odczyt
          Serial.println("  Updating display");
          DisplayData(clClient.currentSg, lastSg, clClient.currentSgDatetime, lastSgDatetime, clClient.currentTrend);
          lastSgDatetimeChange = currentTime;
          lastSgDatetime = clClient.currentSgDatetime;
          lastSg = clClient.currentSg;
        } else {
          // mamy 0
          Serial.println("  Stale data (Current SG = 0)"); //TODO: Filter out single readings with 0
          display.BlueError();
        }
        // koniec recovery
        isAfterError = false;
      } else {
        // nic do sprawdzenia
        Serial.println("  No new readings");
      }
      // brak nowych odczytów przez 10 min
      if (currentTime - lastSgDatetimeChange > 600000) {
        Serial.println("  Stale data (>10min)");
        display.BlueError();
      }

    } else {
      if (clClient.IsConnected()) {
        Serial.println("  CareLink API error");
        display.RedError();
      } else {
        Serial.println("  WiFi connection lost");
        display.YellowError();
      }
      isAfterError = true;
    }
    
    previousTime = currentTime;
    newConnection = false;
  }
}

void PrepareConfig()
{
  Serial.println("Reading configuration...");
  if (!config.IsSet()){
    Serial.println("Configuration empty, entering setup");
    config.SetDefaults();
    display.SetBrightness(config.brightness);   
    display.Logo(); 
    config.Update();
    Serial.println("Configuration updated");
    ESP.restart();
  } else {
    config.Read();
    config.Print();
    Serial.print("Press any key for setup (in 5 seconds)... ");
    display.SetBrightness(config.brightness);
    display.Logo();
    unsigned long startTime = millis();
    while (1) {
      if (Serial.available()) {
        Serial.println("Entering setup");
        config.Update();
        Serial.println("Configuration updated");
        ESP.restart();
        break;
      }
      unsigned long now = millis();
      if (now - startTime >= 5000) {
        Serial.println("Skipped");
        display.ClearDisplay();        
        break;
      }
    }    
  }  
}

bool PeriodicGetData() {
  if (!clClient.GetData()) {    
    if (!clClient.Login()) {      // If GetData failed
      return false;               // And if Login failed - error
    } else {
      if (!clClient.GetData()) {  // If Login succeded
        return false;             // But GetData still fails - error
      }
    }
  }
  
  return true;                    // GetData succeded
}

void PrintData()
{
  Serial.printf("CurrentTrend: %s  CurrentSg: %d  LastSG: %d  CurrentSgDatetime: %lu  LastSgDatetime: %lu  CurrentTime: %lu  LastSgDatetimeChange: %lu  IsAfterError: %s\n", clClient.currentTrend, clClient.currentSg, lastSg, clClient.currentSgDatetime, lastSgDatetime, currentTime, lastSgDatetimeChange, isAfterError ? "true" : "false");
}

void DisplayData(int currentSg, int previousSg, unsigned long currentSgDatetime, unsigned long previousSgDatetime, const String& currentTrend) 
{  
  if (currentSg >= 400) {
    Serial.println("  Current SG over 400");
    display.TooHighArrow(currentSg);
  } else if (currentSg <= 40) {
    Serial.println("  Current SG under 40");
    display.TooLowArrow(currentSg);
  } else if(previousSg == 0) {
    Serial.println("  No last SG, using arrow from current trend");
    display.ArrowFromTrend(currentTrend, currentSg);
  } else if(currentSgDatetime <= previousSgDatetime) {
    Serial.println("  Wrong timestamp, using arrow from current trend");
    display.ArrowFromTrend(currentTrend, currentSg);
  } else {
    float calculatedTrend = CalculateTrend(currentSg, previousSg, currentSgDatetime, previousSgDatetime);
    if (calculatedTrend >= ThreeArrowDifference) {
      display.TripleUpArrow(currentSg);
    } else if (calculatedTrend >= TwoArrowDifference) {
      display.DoubleUpArrow(currentSg);
    } else if (calculatedTrend >= OneArrowDifference) {
      display.UpArrow(currentSg);
    } else if (calculatedTrend > -OneArrowDifference) {
      display.StableArrow(currentSg);
    } else if (calculatedTrend > -TwoArrowDifference) {
      display.DownArrow(currentSg);
    } else if (calculatedTrend > -ThreeArrowDifference) {
      display.DoubleDownArrow(currentSg);
    } else if (calculatedTrend <= -ThreeArrowDifference) {
      display.TripleDownArrow(currentSg);
    } else {
      display.ClearDisplay();
    }
  }
}

float CalculateTrend(int sg, int previousSg, unsigned long datetime, unsigned long previousDatetime) {
  if(datetime < previousDatetime) {
    Serial.println("  Current SG datetime older than last SG datetime, no slope calculation");
    return 0;
  }
  int deltaSg = sg - previousSg;
  Serial.printf("  DeltaSg: %d  ", deltaSg);
  int deltaTimeMinutes = (datetime - previousDatetime) / 60;
  Serial.printf("DeltaTime: %d  ", deltaTimeMinutes);
  float slopePer5min = (deltaSg * 5.0) / deltaTimeMinutes;
  Serial.printf("Slope: %f\n", slopePer5min);
  return slopePer5min;
}

