#include <ESP32Servo.h> 
Servo myservo; 



void servoGo(int pin, int value){
  int pos; 
  //Serial.println("going"); 
  if (value > 0){
    for (pos = 0; pos <= value; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);    // tell servo to go to position in variable 'pos'         // waits 15ms for the servo to reach the position
    delay(15); 
    //Serial.println("next pos"); 
    }
  }
  if (value < 0){
    for (pos = 0; pos <= value; pos -= 1) { // goes from 0 degrees to -180 degrees
    // in steps of 1 degree
    myservo.write(pos);    // tell servo to go to position in variable 'pos'         // waits 15ms for the servo to reach the position
    delay(15); 
    //Serial.println("next pos"); 
    }
  }
  
  
}


void servoInit(int pin){
    // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);    // standard 50 hz servo
  myservo.attach(pin, 1000, 2000); 
  
  servoGo(pin, 0); 

}