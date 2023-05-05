// libraries 
#include <Adafruit_MPU6050.h>
#include "sensors.h"

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
const int luminosityPin = 34; // io pin for the luminosity sensor 

// MPU6050 uses I2C: SDA (21) and SCL (22)
bool accelerometer = true; 
bool gyroscope = true; 
bool temperature = false; 


/*
// some setup for the MPU6050 
void setupMPU6050(){
    if (statusAccelerometer or statusAccelerometer or statusTemperature){
        Serial.println("Adafruit MPU6050 test!");
        if (!mpu.begin()) {
            Serial.println("Failed to find MPU6050 chip");
            while (1) {
                delay(10);
            }
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
*/

// get luminosity 
// 10 millisecond averaging loop 
int getLuminosity(void) {
    int values[5];\
    for (int i=0; i < 5; i++){
        int newValue = analogRead(luminosityPin);
        values[i] = newValue; 
        delay(2); // delay is 2 milliseconds 
    }
    int lumValue = 0; 
    for (int i = 0; i < 5; i++){
        lumValue = lumValue + values[i]; 
    }
    // TODO: normalize the value to a range of 0-100
    float lumAvg = (lumValue/5); // dividing by 5*10. 5 comes from 5 items averaging, 10 is to round the value 
    Serial.print("sensor reading: "); 
    Serial.println(lumAvg); 
    
    return (lumAvg);
}

// get distance 
void getDistance(){
    //  send pulse
    long duration, inches, cm;
    pinMode(pingPin, OUTPUT);
    digitalWrite(pingPin, LOW);
    delayMicroseconds(2);
    digitalWrite(pingPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(pingPin, LOW);
    pinMode(echoPin, INPUT);
    duration = pulseIn(echoPin, HIGH);

    // format value 
    inches = (duration / 74 / 2);
    cm = (duration / 29 / 2);
    Serial.print(inches);
    Serial.print("in, ");
    Serial.print(cm);
    Serial.print("cm");
    Serial.println();
}


// get temperature 
// outputs degrees C (not sure why this would be useful but it's functionality we have access to)
void getTemperature(){
    sensors_event_t temp;
    mpu_temp->getEvent(&temp);
    Serial.print("\t\tTemperature ");
    Serial.print(temp.temperature);
    Serial.println(" deg C"); 
}

// get accelerometer 
// outputs meters/second^2
Accelerometer getAccelerometer(bool x = true, bool y = true, bool z = true) {
  Accelerometer accel;
  sensors_event_t s_event;

  mpu_accel->getEvent(&s_event);
  accel.x = s_event.acceleration.x;
  accel.y = s_event.acceleration.y;
  accel.z = s_event.acceleration.z;

  // TODO: use the global debug variable
#ifdef DEBUG
  {
    Serial.print("\t\tAccel: ");
    if (x) {
      Serial.print(" X : ");
      Serial.print(s_event.acceleration.x);
    }
    if (y){
      Serial.print(" \tY: ");
      Serial.print(s_event.acceleration.y);
    }
    if (z){
      Serial.print(" \tZ: ");
      Serial.print(s_event.acceleration.z);
    }
    Serial.println(" m/s^2 ");
  }
#endif
  return accel;
}


// get gyroscope
// outputs radians/second
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



