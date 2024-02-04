#include <ESP32Servo.h> 

// 9g servo defaults
#define DEFAULT_uS_LOW 400
#define DEFAULT_uS_HIGH 2400

Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;

void servoGo(int which, int value){
  switch(which) {
    case 1:
      servo1.write(value);
      break;
    case 2:
      servo2.write(value);
      break;
    case 3:
      servo3.write(value);
      break;
    case 4:
      servo4.write(value);
      break;
    default:
      Serial.println("Invalid servo number" + which);
      break;
  }
  return;
}

// initialize all servos
void servoInit() {
  // TODO: move servo pins assignment to config file
  servo1.setPeriodHertz(50);    // standard 50 hz servo
  servo2.setPeriodHertz(50);    // standard 50 hz servo
  servo3.setPeriodHertz(50);    // standard 50 hz servo
  servo4.setPeriodHertz(50);    // standard 50 hz servo

  servo1.attach(12, DEFAULT_uS_LOW, DEFAULT_uS_HIGH);
  servo2.attach(32, DEFAULT_uS_LOW, DEFAULT_uS_HIGH);
  servo3.attach(25, DEFAULT_uS_LOW, DEFAULT_uS_HIGH);
  servo4.attach(33, DEFAULT_uS_LOW, DEFAULT_uS_HIGH);
}