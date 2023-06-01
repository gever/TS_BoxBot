/*
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
*/

int getLuminosity1();
int getLuminosity2();
int getDistance(); 
int getTemperature(); 
//int findLine(); 
bool detectLine(int lineval); 
float getAccel_x(); 
float getAccel_y(); 
float getAccel_z(); 
void setupAccel(); 
void testMove(); 
///Gyroscope getGyroscope();
