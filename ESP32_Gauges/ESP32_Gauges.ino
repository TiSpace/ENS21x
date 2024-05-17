/*********
  Rui Santos
  Complete instructions at https://RandomNerdTutorials.com/esp32-web-server-gauges/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.


  //https://randomnerdtutorials.com/esp32-access-point-ap-web-server/
*********/

#include <Arduino.h>
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>


#include <Wire.h>
#include <ens212.h>




// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// Json Variable to Hold Sensor Readings
JSONVar readings;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

using namespace ScioSense;

ENS212 ens212;

#define PIN_SDA 40
#define PIN_SCL 39

float temperatureCelsius = 0;
;
float humidityPercent = 0;


// Get Sensor Readings and return JSON object
String getSensorReadings() {
  readings["temperature"] = String(temperatureCelsius);
  readings["humidity"] = String(humidityPercent);
  String jsonString = JSON.stringify(readings);
  return jsonString;
}

// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin()) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Initialize WiFi
void initWiFi() {


  // Connect WiFi
  WiFiManager wifiMulti;
  Serial.println("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  //WiFi.softAP(ssid, password);
  while (!wifiMulti.autoConnect("ENS21x Interface")) {  // Wait for the Wi-Fi to connect
    delay(250);
    Serial.print('.');
  }
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());  // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());  // Send the IP address of the ESP8266 to the computer
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  delay(2000);
  Serial.println(__FILE__);

  Wire.begin(PIN_SDA, PIN_SCL);
  // Wire.begin();
  ens212.begin();

  if (ens212.isConnected() == false) {
    Serial.println("Error -- The ENS212 is not connected.");
    while (1)
      ;
  }



  initWiFi();
  initSPIFFS();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.serveStatic("/", SPIFFS, "/");

  // Request for the latest sensor readings
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = getSensorReadings();
    request->send(200, "application/json", json);
    json = String();
  });

  events.onConnect([](AsyncEventSourceClient *client) {
    if (client->lastId()) {
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);

  // Start server
  server.begin();
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    // Send Events to the client with the Sensor Readings Every 10 seconds
    events.send("ping", NULL, millis());
    events.send(getSensorReadings().c_str(), "new_readings", millis());
    lastTime = millis();


    if (ens212.singleShotMeasure() == ENS212::Result::STATUS_OK) {
      temperatureCelsius = ens212.getTempCelsius();
      humidityPercent = ens212.getHumidityPercent();

      Serial.print("Temperature:");
      Serial.print(temperatureCelsius);
      Serial.print("°C\t");

      Serial.print("Humidity:");
      Serial.print(humidityPercent);
      Serial.println("%");
    }
  }
}
