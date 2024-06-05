#include <Arduino.h>
#include "tr235.h"




void portExp::LED(uint8_t statusLED) {  //Startbedingung ist 0xFF = alles au

    // for (uint8_t i; i < 4; i++) {
    digitalWrite(PIN_LED_RED, 1 - ((statusLED & RED) >> (RED - 1)));
    digitalWrite(PIN_LED_GREEN, 1 - ((statusLED & GREEN) >> (GREEN - 1)));
    digitalWrite(PIN_LED_BLUE, 1 - ((statusLED & BLUE) >> (BLUE - 2)));


    // }
}



void portExp::init() {

    pinMode(PIN_LED_RED, OUTPUT);
    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_LED_BLUE, OUTPUT);

    pinMode(PIN_BTN1, INPUT_PULLUP);
    pinMode(PIN_BTN2, INPUT_PULLUP);
    pinMode(PIN_BTN3, INPUT_PULLUP);
    pinMode(PIN_BTN4, INPUT_PULLUP);

    pinMode(PIN_EN_MEAS, OUTPUT);
    digitalWrite(PIN_EN_MEAS, HIGH);

    digitalWrite(PIN_LED_RED, HIGH);
    digitalWrite(PIN_LED_GREEN, HIGH);
    digitalWrite(PIN_LED_BLUE, HIGH);

    Serial.println("Init erfolgt");
}


uint8_t portExp::allButton() {
    // any pressed button will return a correspongins 1

    uint8_t readPhysicalButton;
    
    readPhysicalButton = (1 - digitalRead(PIN_BTN1)) * 1 + (1 - digitalRead(PIN_BTN2)) * 2 + (1 - digitalRead(PIN_BTN3)) *4 + (1 - digitalRead(PIN_BTN4)) * 8;

    return readPhysicalButton;
}

uint8_t portExp::readButton(uint8_t buttonNr) {  // returns 1 if corresponding button is pressed

      return  ((this->allButton() & (1 << buttonNr)) >> buttonNr);
}


void portExp::waitButton_Press(uint8_t buttonNr)
// waits and return once the button is pressed
{
      while (((this->allButton() & (1 << buttonNr)) >> buttonNr) == 0) {}  //button pressed
}


void portExp::waitButton_PressAndRelease(uint8_t buttonNr)
// waits and return once the button is pressed and released
{
     while (((this->allButton() & (1 << buttonNr)) >> buttonNr) == 0) {}  //button pressed
     while (((this->allButton() & (1 << buttonNr)) >> buttonNr) == 1) {}  //button released
}

float portExp::battery() {
    uint16_t readValue = 0;
    digitalWrite(PIN_EN_MEAS, HIGH);  //turn on ADC voltage devider
    for (int i = 0; i < 10; i++) {    // build average of xx measures
        readValue += analogRead(PIN_ADC);
        delay(10);
    }
    return readValue / 10 * ADC_REF_V / 8191 * 2;
    digitalWrite(PIN_EN_MEAS, LOW);
    return 34.567;
}



//}