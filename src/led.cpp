#include "led.h"
#include "main.h"

void ledGo(int pin, bool status){
    pinMode (pin, OUTPUT); 

    if (status == true){
        digitalWrite(pin, HIGH); 
    }
    if (status == false){
        digitalWrite(pin, LOW); 
    }
}