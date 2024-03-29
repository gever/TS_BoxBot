// libraries 
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "sensors.h"
#include "main.h"

Adafruit_MPU6050 mpu;
Adafruit_Sensor *mpu_temp, *mpu_accel, *mpu_gyro;

// setup the status of the sensors
bool statusecho = false;
bool statusLuminosity = true; 
bool statusAccelerometer = false; 
bool statusGyroscope = false; 
bool statusTemperature = false; 

// allocate pins 
const int pingPin = 5;  // Trigger Pin of Ultrasonic Sensor
const int echoPin = 18; // Echo Pin of Ultrasonic Sensor
const int luminosityPin1 = 32; // Labeled "Servo 2" on the carrier board 
const int luminosityPin2 = 33; // Labeled "Servo 4" on the carrier board 

// MPU6050 uses I2C: SDA (21) and SCL (22)
bool accelerometer = true; 
bool gyroscope = true; 
bool temperature = false; 

void sensors_setup() {
  // setup the sensors 
  setupAccel();
  pinMode(luminosityPin1, INPUT_PULLDOWN);
  pinMode(luminosityPin2, INPUT_PULLDOWN);
}

// LUMINOSITY
// take an average of N samples
bool sampleLuminosity(int which) {
  // treat the pin as a digital input
  // average N digital samples
  int N = 4;
  int sum = 0;
  for (int i = 0; i < N; i++) {
    sum += digitalRead(which);
  }
  return sum >= N/2;
}

int getLuminosity1(void) {
    return sampleLuminosity(luminosityPin1);
}

int getLuminosity2(void) {
    return sampleLuminosity(luminosityPin2);
}

// get distance 
int getDistance(void){
    //  send pulse
    long duration;
    float cm;
    pinMode(pingPin, OUTPUT);
    digitalWrite(pingPin, LOW);
    delayMicroseconds(2);
    digitalWrite(pingPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(pingPin, LOW);
    pinMode(echoPin, INPUT);
    duration = pulseIn(echoPin, HIGH);

    // format value 
    // inches = (duration / 74 / 2);
    // how long for the ping to go out and come back?
    cm = (duration / 29.0) / 2.0;
    // Serial.print(inches);
    // Serial.print("in, ");
    // Serial.print(cm);
    // Serial.print("cm");
    // Serial.println();

    return(cm);
}


void setupAccel() {
  // Accelerometer 
  if (false){ // making this false so that the acceleromter doesn't go off THIS IS A PROBLEM TO RESOLVE
      Serial.println("Adafruit MPU6050 test!");
    if (!mpu.begin()) {
      Serial.println("Failed to find MPU6050 chip");
      /* I think this would make it loop indefinitely!!! badddd 
      while (1) {
        delay(10);
      }
      */
    }

    Serial.println("MPU6050 Found!");
      mpu_temp = mpu.getTemperatureSensor();
      mpu_temp->printSensorDetails();

      mpu_accel = mpu.getAccelerometerSensor();
      mpu_accel->printSensorDetails();

      mpu_gyro = mpu.getGyroSensor();
      mpu_gyro->printSensorDetails();
    }
  }

// get temperature 
// outputs degrees C (not sure why this would be useful but it's functionality we have access to)
int getTemperature(void){
    sensors_event_t temp;
    mpu_temp->getEvent(&temp);
    Serial.print("\t\tTemperature ");
    Serial.print(temp.temperature);
    Serial.println(" deg C"); 
    return (temp.temperature); 
}

float getAccel_x() {
  mpu_accel = mpu.getAccelerometerSensor();
  sensors_event_t accel;
  mpu_accel->getEvent(&accel); 
  float v = accel.acceleration.x;
  return v; 
}

float getAccel_y() {
  mpu_accel = mpu.getAccelerometerSensor();
  sensors_event_t accel;
  mpu_accel->getEvent(&accel); 
  float v = accel.acceleration.y;
  Serial.print(v); 
  return v;
}

float getAccel_z() {
  mpu_accel = mpu.getAccelerometerSensor();
  sensors_event_t accel;
  mpu_accel->getEvent(&accel); 
  float v = accel.acceleration.z;
  Serial.print(v);
  return v;
}



/*
Gyroscope getGyroscope(){
    sensors_event_t s_event;
    Gyroscope gyro;

    mpu_gyro->getEvent(&s_event);
    gyro.x = s_event.gyro.x;
    gyro.y = s_event.gyro.y;
    gyro.z = s_event.gyro.z;

    // TODO: use the global debug variable
#ifdef DEBUG
    {
      Serial.print("\t\tGyro: ");
      if (x){
        Serial.print(" X : ");
        Serial.print(s_event.gyro.x);
      }
      if (y){
        Serial.print(" \tY: ");
        Serial.print(s_event.gyro.y);
      }
      if (z){
        Serial.print(" \tZ: ");
        Serial.print(s_event.gyro.z);
      }
      Serial.println(" radians/s ");
    }
#endif

    return gyro;
}

*/
// get gyroscope
// outputs radians/second



