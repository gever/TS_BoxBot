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
void getDistance(); 
void getTemperature(); 
Accelerometer getAccelerometer(); 
Gyroscope getGyroscope();
