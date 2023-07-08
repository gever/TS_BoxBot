#include <ESP32Servo.h> 
Servo myservo; 



void servoGo(int pin, int value){
  int pos; 
  Serial.print("going: ");
  Serial.println(value);

  myservo.write(value);
  return;
}


void servoInit(int pin){
  myservo.setPeriodHertz(50);    // standard 50 hz servo
  myservo.attach(pin, 1000, 2000); 
  
  // servoGo(pin, 0); 

}