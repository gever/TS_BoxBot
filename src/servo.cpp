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
  // proper initialization for 9g servo
  //#define DEFAULT_uS_LOW 400
  //#define DEFAULT_uS_HIGH 2400
  myservo.attach(pin, 400, 2400); 
  
  // servoGo(pin, 0); 

}