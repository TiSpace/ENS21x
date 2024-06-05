/************************************************************************************************************
  ENS21x Webserver
  #author: tr 05/24
  #hardware: ESP32-S2 mini

  #credits: based on the instruction from  Rui Santos (https://RandomNerdTutorials.com/esp32-web-server-gauges/)
  
  
*************************************************************************************************************/

#include <Arduino.h>
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>

#include <Wire.h>
#include <ens212.h>  // https://www.sciosense.com/

#include "setting.h"
#include "tr235.h"

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
portExp uIO;


float temperatureCelsius = 0;
float humidityPercent = 0;
float batteryVoltage = 0;


/* ================================================================================
             S E T U P
   ================================================================================
*/
void setup() {
    uIO.init();

    uIO.LED(BLUE);

    //     WiFi.mode(WIFI_STA);
    // WiFi.begin("Andromeda","Plejaden");
    initWiFi();
    initSPIFFS();


    uIO.LED(RED);
    // Serial port for debugging purposes
    Serial.begin(BAUDRATE);
    delay(2000);
    Serial.println(__FILE__);
    Serial.print(__DATE__);
    Serial.print("\t");
    Serial.println(__TIME__);
    Serial.print("MAC: ");
    Serial.print((String)WiFi.macAddress());
    Serial.print("\t");
    Serial.print((String)WiFi.RSSI());
    Serial.print("dB at SSID ");
    Serial.println((String)WiFi.SSID());
    Serial.print("\n\n");

    Wire.begin(PIN_SDA, PIN_SCL);

    ens212.begin();

    if (ens212.isConnected() == false) {
        Serial.println("Error -- The ENS212 is not connected.");
        checkI2C();
        while (1)
            ;
    }


    uIO.LED(OFF);
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

/* ================================================================================
              L O O P
   ================================================================================
*/
void loop() {
    if ((millis() - lastTime) > timerDelay) {
        // Send Events to the client with the Sensor Readings Every 10 seconds
        events.send("ping", NULL, millis());
        events.send(getSensorReadings().c_str(), "new_readings", millis());
        lastTime = millis();


        if (ens212.singleShotMeasure() == ENS212::Result::STATUS_OK) {
            uIO.LED(GREEN);
            temperatureCelsius = ens212.getTempCelsius();
            humidityPercent = ens212.getHumidityPercent();
            batteryVoltage = uIO.battery();
            Serial.print("Temperature:");
            Serial.print(temperatureCelsius);
            Serial.print("°C\t");

            Serial.print("Humidity:");
            Serial.print(humidityPercent);
            Serial.print("%\t\t");

            Serial.print("Battery: ");
            Serial.print(batteryVoltage, 2);
            Serial.print("V\t");



            Serial.print("\n");




            delay(100);
            uIO.LED(OFF);
        }
    }
}



/* ================================================================================
            Get Sensor Readings and return JSON object
   ================================================================================
*/

String getSensorReadings() {
    readings["temperature"] = String(temperatureCelsius);
    readings["humidity"] = String(humidityPercent);
    readings["battery"] = String(batteryVoltage);
    readings["file"] = String(__FILE__);
    readings["time"] = String(__TIME__);
    readings["date"] = String(__DATE__);
    readings["MAC"]=(String)WiFi.macAddress();
    String jsonString = JSON.stringify(readings);
    return jsonString;
}



/* ================================================================================
            Initialize SPIFFS
   ================================================================================
*/
void initSPIFFS() {
    if (!SPIFFS.begin()) {
        Serial.println("An error has occurred while mounting SPIFFS");
    }
    Serial.println("SPIFFS mounted successfully");
}


/* ================================================================================
            Initialize WiFi
   ================================================================================
*/
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


/* ================================================================================
            Initialize LED
   ================================================================================
*/
// void initLED() {
//     pinMode(PIN_LED_RED, OUTPUT);
//     pinMode(PIN_LED_GREEN, OUTPUT);
//     pinMode(PIN_LED_BLUE, OUTPUT);

//     pinMode(PIN_BTN1, INPUT_PULLUP);
//     pinMode(PIN_BTN2, INPUT_PULLUP);
//     pinMode(PIN_BTN3, INPUT_PULLUP);
//     pinMode(PIN_BTN4, INPUT_PULLUP);

//     digitalWrite(PIN_LED_RED, HIGH);
//     digitalWrite(PIN_LED_GREEN, HIGH);
//     digitalWrite(PIN_LED_BLUE, HIGH);

//     digitalWrite(PIN_LED_RED, LOW);
//     delay(100);
//     digitalWrite(PIN_LED_RED, HIGH);
//     delay(50);
//     digitalWrite(PIN_LED_BLUE, LOW);
//     delay(100);
//     digitalWrite(PIN_LED_BLUE, HIGH);
//     delay(50);
//     digitalWrite(PIN_LED_GREEN, LOW);
//     delay(100);
//     digitalWrite(PIN_LED_GREEN, HIGH);
//     delay(50);
// }



/* ================================================================================
            check valid I²C devices
   ================================================================================
*/
void checkI2C() {
    uint8_t error, address;
    int nDevices;

    Serial.println("Scanning...");

    nDevices = 0;
    for (address = 1; address < 127; address++) {
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0) {
            Serial.print("I2C device found at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.print(address, HEX);
            Serial.println("  !");

            nDevices++;
        } else if (error == 4) {
            Serial.print("Unknown error at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.println(address, HEX);
        }
    }
    if (nDevices == 0)
        Serial.println("No I2C devices found\n");
    else
        Serial.println("done\n");

    delay(5000);  // wait 5 seconds for next scan
}
