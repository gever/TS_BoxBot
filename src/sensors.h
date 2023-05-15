typedef struct {
  int x;
  int y;
  int z;
} Accelerometer;

typedef struct {
    int x;
    int y;
    int z;
} Gyroscope;

int getLuminosity();
int getDistance(); 
int getTemperature(); 
Accelerometer getAccelerometer(); 
Gyroscope getGyroscope();
