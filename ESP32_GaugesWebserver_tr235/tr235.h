#ifndef tr235_H_
#define tr235_H_

#include <Arduino.h>



// LED
#define OFF 0
#define GREEN 1
#define RED 2
#define BLUE 4

//buttons
#define buttonA 0
#define buttonB 1
#define buttonC 2
#define buttonD 3


// Pin definitions
#define PIN_LED_RED       38
#define PIN_LED_GREEN     37
#define PIN_LED_BLUE      35

#define PIN_BTN1          17
#define PIN_BTN2          16
#define PIN_BTN3          21
#define PIN_BTN4          18

#define PIN_EN_MEAS       5
#define PIN_ADC           6
#define   ADC_REF_V       2.625 // Reference voltage ADC (?)

class portExp {
public:

  void init();
  void LED(uint8_t statusLED);
  float battery();
  
  uint8_t allButton();
  uint8_t readButton(uint8_t buttonNr);
  void waitButton_Press(uint8_t buttonNr);
  void waitButton_PressAndRelease(uint8_t buttonNr);


private:


};



#endif /* tr235_H_ */