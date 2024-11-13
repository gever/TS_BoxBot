#include "led.h"
#include "Arduino.h"

void ledGo(int pin, bool status) {

    // Pin 6 to 11 are reserved and 34, 35, 36, 39 are used for input only.
    // Pin 2 will light up the onboard LED.
    if ((pin >= 6 && pin <= 11) || pin == 34 || pin == 35 || pin == 36 || pin == 39) {
      return;
    }

    pinMode (pin, OUTPUT); 

    if (status) {
        digitalWrite(pin, HIGH); 
    } else {
        digitalWrite(pin, LOW); 
    }
}
