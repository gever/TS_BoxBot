#include <ESP32Servo.h> 
Servo myservo; 

void servoSetup(int servoPin){
  Serial.println("getting started with servo!"); 
  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);    // standard 50 hz servo
  myservo.attach(servoPin, 1000, 2000); // attaches the servo on pin 18 to the servo object
  // using default min/max of 1000us and 2000us
  // different servos may require different min/max settings
  // for an accurate 0 to 180 sweep
}

void servoMove (int value){ 
  int pos = 0; 
  Serial.println("going"); 
  for (pos = 0; pos <= value; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);    // tell servo to go to position in variable 'pos'         // waits 15ms for the servo to reach the position
    delay(15); 
    Serial.println("next pos"); 
  }
  for (pos = value; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);    // tell servo to go to position in variable 'pos'
    delay(15); 
  }
}
