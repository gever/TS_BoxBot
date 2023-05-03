// libraries 
#include <Adafruit_MPU6050.h>

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
bool x = true; 
bool y = true; 
bool z = true; 

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
int getLuminosity(){
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
    float lumAvg = (lumValue/5); // dividing by 5*10. 5 comes from 5 items averaging, 10 is to round the value 
    Serial.println(lumAvg); 
    Serial.println("sensor reading:"); 
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
void getAccelerometer(){
    sensors_event_t accel;
    mpu_accel->getEvent(&accel);
    Serial.print("\t\tAccel: ");
    if (x){
      Serial.print(" X : ");
      Serial.print(accel.acceleration.x);
    }
    if (y){
      Serial.print(" \tY: ");
      Serial.print(accel.acceleration.y);
    }
    if (z){
      Serial.print(" \tZ: ");
      Serial.print(accel.acceleration.z);
    }
    Serial.println(" m/s^2 ");
}


// get gyroscope
// outputs radians/second
void getGyroscope(){
    sensors_event_t gyro;
    mpu_gyro->getEvent(&gyro);
    Serial.print("\t\tGyro: ");
    if (x){
      Serial.print(" X : ");
      Serial.print(gyro.gyro.x);
    }
    if (y){
      Serial.print(" \tY: ");
      Serial.print(gyro.gyro.y);
    }
    if (z){
      Serial.print(" \tZ: ");
      Serial.print(gyro.gyro.z);
    }
    Serial.println(" radians/s ");
}



