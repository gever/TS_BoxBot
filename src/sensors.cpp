// libraries
#include "sensors.h"
#include "main.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>

#ifdef NOT_USED
Adafruit_MPU6050 mpu;
Adafruit_Sensor *mpu_temp, *mpu_accel, *mpu_gyro;

// setup the status of the sensors
bool statusecho = false;
bool statusLuminosity = true;
bool statusAccelerometer = false;
bool statusGyroscope = false;
bool statusTemperature = false;
// MPU6050 uses I2C: SDA (21) and SCL (22)
bool accelerometer = true;
bool gyroscope = true;
bool temperature = false;
#endif // NOT_USED

#define DIST_SAMPLE_RATE 8
volatile int currentDistance = 0;
TaskHandle_t distanceTaskHandle = NULL;

// allocate pins
const int pingPin = 5;         // Trigger Pin of Ultrasonic Sensor
const int echoPin = 18;        // Echo Pin of Ultrasonic Sensor
const int luminosityPin1 = 32; // Labeled "Servo 2" on the carrier board
const int luminosityPin2 = 33; // Labeled "Servo 4" on the carrier board

bool sensors_initialized = false;

// forward declaration
int readDistanceSensor(void);

void distanceTask(void *parameter) {
  for (;;) {
    currentDistance = readDistanceSensor();
    vTaskDelay(1000 / DIST_SAMPLE_RATE / portTICK_PERIOD_MS);
  }
}

void sensors_setup() {
  if (sensors_initialized) {
    return;
  }
  // setup the sensors
  // setupAccel();
  pinMode(luminosityPin1, INPUT_PULLDOWN);
  pinMode(luminosityPin2, INPUT_PULLDOWN);

  // set up the distance sensor task
  // Core 0 is usually where the WiFi stuff runs? Use 1?
  // Standard ESP32 Arduino: loop() is on Core 1. WiFi/BT on Core 0.
  // User asked to "run the distance sensor on a different core".
  // If webserver (WiFi) is on 0, and loop is on 1, we should probably run this
  // on 0 if we want it off the main loop, but if WiFi is busy it might jitter.
  // Wait, "On the webserver core, we just return the current value".
  // This implies the webserver is running and we want to unblock IT.
  // Webserver runs in the context of `loop()` (via `server.handleClient()`) OR
  // on Core 0 (async). The User's `main.cpp` calls `server.handleClient()`
  // inside `loop()`. `loop()` runs on Core 1 by default in Arduino ESP32. So
  // the webserver is effectively running on Core 1. So we should put the sensor
  // on Core 0.
  xTaskCreatePinnedToCore(
      distanceTask,        /* Task function. */
      "DistanceTask",      /* name of task. */
      10000,               /* Stack size of task */
      NULL,                /* parameter of the task */
      1,                   /* priority of the task */
      &distanceTaskHandle, /* Task handle to keep track of created task */
      0);                  /* pin task to core 0 */

  sensors_initialized = true;
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
  return sum >= N / 2;
}

int getLuminosity1(void) {
  sensors_setup();
  return sampleLuminosity(luminosityPin1);
}

int getLuminosity2(void) {
  sensors_setup();
  return sampleLuminosity(luminosityPin2);
}

// get distance
// This is now just returning the shared variable
int getDistance(void) {
  sensors_setup();
  return currentDistance;
}

// Internal function to read the hardware
int readDistanceSensor(void) {
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
  duration = pulseIn(echoPin, HIGH, 25000);

  // format value
  // inches = (duration / 74 / 2);
  // how long for the ping to go out and come back?
  cm = (duration / 29.0) / 2.0;
  // Serial.print(inches);
  // Serial.print("in, ");
  // Serial.print(cm);
  // Serial.print("cm");
  // Serial.println();

  return (cm);
}

#ifdef NOT_USED
void setupAccel() {
  // Accelerometer
  if (false) { // making this false so that the acceleromter doesn't go off THIS
               // IS A PROBLEM TO RESOLVE
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
// outputs degrees C (not sure why this would be useful but it's functionality
// we have access to)
int getTemperature(void) {
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
#endif // NOT_USED

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
