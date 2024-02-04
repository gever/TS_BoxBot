/*
  based on WiFiAccessPoint.ino by Elochukwu Ifediora (fedy0)
*/

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>
// #include <mdns.h>
#include "util.h"
#include "sensors.h"
#include "servo.h"
#include "led.h"

// Display includes and defs
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define SDEBUG(label, x) \
  {                      \
    Serial.print(label); \
    Serial.println(x);   \
  }
#define ABS(x) ((x < 0) ? (-x) : (x))

WebServer server(80);

// things that can be set through settings UI
// TODO: store these in flash filesystem
uint16_t motor_step_rate = 950;
float linear_turn_fudge = 1.0;
float linear_motion_fudge = 1.0;
bool wheels_forward = true;

// Set these to your desired credentials.
bool use_wifi = false;
const char *ssid = "wacky-squirrel";
const char *password = (char *)NULL;

// primitive motion plan parsing/interpreting
#define MAX_PLAN_LEN 1024
char *plan_token = NULL;
bool plan_ready = false;
char plan_buffer[MAX_PLAN_LEN];
char *plan = NULL;
const char delim[] = ",\n";

// hardware clock to step the motors at a good rate
hw_timer_t *step_timer = NULL;

const bool FWD = true;
const bool BWD = false;

#define DW(p, v) digitalWrite(p, v)
#define SET_BITS(a, b, c, d) \
  {                          \
    DW(pin1, a);             \
    DW(pin2, b);             \
    DW(pin3, c);             \
    DW(pin4, d);             \
  }

class MarsStepper
{
private:
  bool enabled;
  bool dir;
  unsigned short pin1, pin2, pin3, pin4;
  unsigned int step_counter;
  unsigned int divisor;

public:
  unsigned int current_step;
  MarsStepper(int motor_pin_1, int motor_pin_2,
              int motor_pin_3, int motor_pin_4)
  {
    pin1 = motor_pin_1;
    pin2 = motor_pin_2;
    pin3 = motor_pin_3;
    pin4 = motor_pin_4;

    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
    pinMode(pin3, OUTPUT);
    pinMode(pin4, OUTPUT);

    current_step = 0; // actual rotation/phase of motor
    step_counter = 0; // keeping track of how often step() function is called
    divisor = 1;      // how often step() calls should be heeded

    enabled = false;
    dir = FWD;
  }
  void set_direction(bool d)
  {
    // SDEBUG("set_direction = ", d);
    dir = d;
  }
  void enable()
  {
    enabled = true;
  }
  void disable()
  {
    enabled = false;
    SET_BITS(0, 0, 0, 0);
  }
  void set_divisor(unsigned int v)
  {
    divisor = v;
  }
  void step()
  {
    // always increment the counter
    step_counter++;

    // don't do anything if we're not running
    if (!enabled)
      return;

    // don't do anything if it's not our turn yet
    if (step_counter % divisor)
      return;

    // step! (activate the next set of stators)
    switch (current_step % 8)
    {
    case 0:
      SET_BITS(1, 0, 0, 0);
      break;
    case 1:
      SET_BITS(1, 1, 0, 0);
      break;
    case 2:
      SET_BITS(0, 1, 0, 0);
      break;
    case 3:
      SET_BITS(0, 1, 1, 0);
      break;
    case 4:
      SET_BITS(0, 0, 1, 0);
      break;
    case 5:
      SET_BITS(0, 0, 1, 1);
      break;
    case 6:
      SET_BITS(0, 0, 0, 1);
      break;
    case 7:
      SET_BITS(1, 0, 0, 1);
      break;
    }
    current_step += (dir ? 1 : -1); // FWD=true, BWD=false
  }
};

// create the motors
MarsStepper m1(13, 14, 27, 26);
MarsStepper m2(15, 2, 4, 19); // this is changed!

const float rot_steps = 2038 * 2;                                                   // one full rotation of the motor shaft
const float boxbot_body_rad = 55;                                                   // the distance from the pen to the wheel in mm
const float boxbot_turn_circ = PI * 2.0 * boxbot_body_rad;                          // circumference of circle described by the wheels
const float boxbot_wheel_rad = 25;                                                  // in mm
const float boxbot_wheel_circ = PI * 2.0 * boxbot_wheel_rad;                        // circumference of the wheel in mm
const float boxbot_turn_steps = (boxbot_turn_circ / boxbot_wheel_circ) * rot_steps; // number of steps to turn 360 deg
const float boxbot_steps_mm = rot_steps / boxbot_wheel_circ;                        // steps per mm of linear motion

void step_the_motors();
void IRAM_ATTR onTimer()
{
  step_the_motors();
}

int step_count = 0; // for the current motion, for all active motors

/*
    setup a stop!
*/

void setup_stop(){
  m2.disable();
  m1.disable();
}


/*
   set up a move command
   dir  - fwd=1, bwd=0
   dist - in centimeters
*/

void setup_move(bool dir, int dist)
{
  // SDEBUG("setup_move\ndir = ", dir);
  // SDEBUG("dist = ", dist);
  step_count = (dist * 10) * boxbot_steps_mm * linear_motion_fudge;
  if (dir)
  {
    // SDEBUG("FWD:", dir);
    m1.set_direction(0);
    m2.set_direction(1);
  }
  else
  {
    // SDEBUG("BWD:", dir);
    m1.set_direction(1);
    m2.set_direction(0);
  }
  m1.enable();
  m2.enable();

  // start movement
  // timerAttachInterrupt(step_timer, &onTimer, true);
}

/*
   set up a turn command
*/
void setup_turn(bool dir, int angle)
{
  step_count = boxbot_turn_steps * (((float)angle) / 360.0f) * linear_turn_fudge;
  if (dir)
  {
    m1.set_direction(1);
    m2.set_direction(1);
  }
  else
  {
    m1.set_direction(0);
    m2.set_direction(0);
  }
  m1.enable();
  m2.enable();

  // start movement
  // timerAttachInterrupt(step_timer, &onTimer, true);
}

void step_the_motors()
{
  if (step_count)
  {
    step_count--;

    m1.step();
    m2.step();

    if (step_count == 0)
    {
      // save power, turn off the motors when not moving
      m1.disable();
      m2.disable();
      // TODO: evaluate possibility of turning off the interrupts while idle (might save power)
    }
  }
}

// configure the interrupt timer
void setup_timer()
{
  // pretty good tutorial on interrupt timers:
  // https://iotespresso.com/timer-interrupts-with-esp32/
  if (step_timer == NULL)
  {
    step_timer = timerBegin(0, 80, true); // configured for 1MHZ (1,000,000/sec)
  }
  timerAttachInterrupt(step_timer, &onTimer, true);
  timerAlarmWrite(step_timer, motor_step_rate, true);
  timerAlarmEnable(step_timer);
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}

// CONSIDER: right now you can interrupt the active movement by
//           sending a new movement command

void handleMove()
{
  if (server.args())
  {
    int v = server.arg(0).toInt(); // negative for backwards movement
    setup_move(v < 0 ? BWD : FWD, ABS(v));
  }
  server.send(200, "application/json", "{\"status\":\"ACK\"}");
}

void handleTurn()
{
  // TODO: clean up the semantics of the turn command
  //       (there is legacy API here that is from the old BU code)
  if (server.args())
  {
    int v = server.arg(0).toInt();
    setup_turn(v < 0 ? 1 : 0, ABS(v)); // negative for left turns
    Serial.println("handleTurn: " + String(v));
  }
  server.send(200, "application/json", "{\"status\":\"ACK\"}");
}

void handleStop()
{
  m1.disable();
  m2.disable();
  step_count = 0;
  plan_ready = false;
  server.send(200, "application/json", "{\"status\":\"ACK\"}");
}

void handleBusy() {
  if (step_count > 0) {
    // Serial.println("handleBusy: busy");
    server.send(200, "application/json", "{\"busy\":true}");
  } else {
    // Serial.println("handleBusy: not busy");
    server.send(200, "application/json", "{\"busy\":false}");
  }
}

/*
 * Sensors
 */
#define JSON_BUFFER_SIZE 128

void handleLuminosity1()
{
  char jsonBuffer[JSON_BUFFER_SIZE];
  int v = getLuminosity1();
  snprintf(jsonBuffer, JSON_BUFFER_SIZE, "{\"luminosity1\":%d}", v);
  server.send(200, "application/json", jsonBuffer);
}

void handleLuminosity2()
{
  char jsonBuffer[JSON_BUFFER_SIZE];
  int v = getLuminosity2();
  snprintf(jsonBuffer, JSON_BUFFER_SIZE, "{\"luminosity2\":%d}", v);
  server.send(200, "application/json", jsonBuffer);
}


void handleDistance()
{
  char jsonBuffer[JSON_BUFFER_SIZE];
  int v = getDistance();
  Serial.print(v);
  snprintf(jsonBuffer, JSON_BUFFER_SIZE, "{\"distance\":%d}", v);
  server.send(200, "application/json", jsonBuffer);
}

void handleAccel_x()
{
  char jsonBuffer[JSON_BUFFER_SIZE];
  float v = getAccel_x();
  snprintf(jsonBuffer, JSON_BUFFER_SIZE, "{\"accel x\":%f}", v);
  server.send(200, "application/json", jsonBuffer);
}

void handleAccel_y()
{
  char jsonBuffer[JSON_BUFFER_SIZE];
  float v = getAccel_y();
  snprintf(jsonBuffer, JSON_BUFFER_SIZE, "{\"accel y\":%d}", v);
  server.send(200, "application/json", jsonBuffer);
}

void handleAccel_z()
{
  char jsonBuffer[JSON_BUFFER_SIZE];
  float v = getAccel_z();
  snprintf(jsonBuffer, JSON_BUFFER_SIZE, "{\"accel z\":%d}", v);
  server.send(200, "application/json", jsonBuffer);
}


void handleServoGo()
{
  if (server.args())
  {
    int servoPin = server.arg(0).toInt(); // pin
    int servoAngle = server.arg(1).toInt(); // angle
    servoGo(servoPin, servoAngle);
  }
  server.send(200, "application/json", "{\"status\":\"ACK\"}");
}

void handleServoInit()
{
  if (server.args())
  {
    int servoPin = server.arg(0).toInt(); // pin
    
  }
  server.send(200, "application/json", "{\"status\":\"ACK\"}");
}

void handleLED()
{
  if (server.args())
  {
    int pin = server.arg(0).toInt(); // pin
    bool status = server.arg(1).toInt(); // status
    ledGo(pin, status);
  }
  server.send(200, "application/json", "{\"status\":\"ACK\"}"); 
}




void handleDetectLine()
{
  char jsonBuffer[JSON_BUFFER_SIZE];
  bool v = detectLine(4095);  // 4095 is for a dark black line on white background
  snprintf(jsonBuffer, JSON_BUFFER_SIZE, "{\"line?\":%s}", v);
  server.send(200, "application/json", jsonBuffer);
}
void handleTemperature()
{
  char jsonBuffer[JSON_BUFFER_SIZE];
  int v = getTemperature();
  snprintf(jsonBuffer, JSON_BUFFER_SIZE, "{\"temperature\":%d}", v);
  server.send(200, "application/json", jsonBuffer);
}

void handleSave()
{
  // TODO: save current settings to SPIFFS/flash memory
  // TODO: save load current settings from SPIFFS
  if (server.args())
  {
    String label;

    label = "motor-step-rate";
    if (server.hasArg(label))
    {
      motor_step_rate = server.arg(label).toInt();
      // TODO: motor_setup( motor_step_rate );
    }
    label = "linear-motion-fudge";
    if (server.hasArg(label))
    {
      linear_motion_fudge = server.arg(label).toFloat();
    }
    label = "linear-turn-fudge";
    if (server.hasArg(label))
    {
      linear_turn_fudge = server.arg(label).toFloat();
    }
  }
  // TODO: handleLandingPage();
}

// parse a string into a number
int parse_int(char *str)
{
  int num = 0;
  int i = 0;
  if (str[i] == '-')
    i++;
  while (str[i] >= '0' && str[i] <= '9')
  {
    num = num * 10 + str[i] - '0';
    i++;
  }
  if (str[0] == '-')
    num *= -1;
  return num;
}

void handlePlan()
{
  if (server.args())
  {
    if (server.arg(0).length() > MAX_PLAN_LEN - 1)
    {
      Serial.println("ERR: Motion plan length exceeds buffer size - ignoring plan.");
      server.send(200, "application/json", "{\"status\":\"ACK\"}"); // TODO: send better return statuses
      return;
    }

    server.send(200, "application/json", "{\"status\":\"ACK\"}"); // TODO: send better return statuses

    strcpy(plan_buffer, server.arg(0).c_str());
    plan = plan_buffer;               // point it back at the front of the buffer
    plan_token = strtok(plan, delim); // get the first token
    plan_ready = true;
  }
}

void serveGenericPage(String url)
{
  Serial.print("handlePageRequest: " + url + " ");
  if (!SPIFFS.exists(url))
  {
    Serial.println("file does not exist:" + url);
    server.send(404, "text/plain", "file does not exist" + url);
    return;
  }

  File file = SPIFFS.open(url, "r");
  if (!file)
  {
    Serial.println("file open failed" + url);
    server.send(404, "text/plain", "file open failed" + url);
    return;
  }
  String contentType = "text/plain";
  if (url.endsWith(".html"))
    contentType = "text/html";
  else if (url.endsWith(".js"))
    contentType = "text/javascript";
  else if (url.endsWith(".css"))
    contentType = "text/css";
  else if (url.endsWith(".ico"))
    contentType = "image/x-icon";
  else if (url.endsWith(".mp3"))
    contentType = "audio/mpeg";
  else if (url.endsWith(".png"))
    contentType = "image/png";
  else if (url.endsWith(".svg"))
    contentType = "image/svg+xml";
  else if (url.endsWith(".jpg"))
    contentType = "image/jpeg";
  else if (url.endsWith(".zip") || url.endsWith(".gz"))
    contentType = "application/javascript";
  Serial.println(contentType);
  server.streamFile(file, contentType);
  file.close();
}

void executePlan()
{
  // only parse/setup next statement in the plan if we have finished the previous step
  if (plan_ready && (step_count == 0))
  {
    int num = 0;
    if (*plan_token == 'M')
    {
      num = parse_int(++plan_token);
      setup_move(num < 0 ? BWD : FWD, num < 0 ? -num : num);
    }
    else if (*plan_token == 'T')
    {
      num = parse_int(++plan_token);
      setup_turn(num < 0 ? 0 : 1, num < 0 ? -num : num);
    }
    else if (*plan_token == 'P')
    {
      num = parse_int(++plan_token);
      // TODO: set_pen_position( num );
    }

    plan_token = strtok(NULL, delim); // set up the next bit of code to execute
    if (plan_token == NULL)
    { // finished the script
      Serial.println("executePlan: end");
      plan_ready = false;
    }
  }
}

// handle request for static page content from the SPIFFS filesystem
void handlePageRequest()
{
  String url = server.uri();
  if (url == "/")
    url = "/index.html";
  serveGenericPage(url);
}

void addAllFiles()
{
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file)
  {
    Serial.print("  FILE: ");
    Serial.println(file.name());
    server.on("/" + String(file.name()), handlePageRequest);
    file = root.openNextFile();
  }
}

// display current status or boot progress
void status_update(const char *msg, bool newline = true)
{
  // TODO: scroll the display if too many messages come out
  if (newline) {
    Serial.println(msg);
    display.println(msg);
  } else {
    Serial.print(msg);
    display.print(msg);
  }
  display.display();
}
// Replace with your network credentials
#include "network_credentials.h"

void setup()
{
  String ip_addr_str = "<not set>";
  bool spiffs_ok = false;

  Serial.begin(115200);
  while (!Serial)
  {
    delay(10);
  } // wait for serial port to connect. Needed for native USB port only
  Serial.println("starting boxbot!!");

  //------------------------------------------
  // Do display stuff
	// initialize with the I2C addr 0x3C
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
 	display.clearDisplay();
 	display.setTextSize(1);
	display.setTextColor(WHITE);
	status_update("boxbot - start");

  // rtc.setTime(30,15,23,2,3,2023); // setup the time (this is for the sensors)
  // Serial.println("\n\nBoxbot v0.6 --------");

  if (!SPIFFS.begin(true))
  {
    status_update("ERR: SPIFFS Mount Failed");
    spiffs_ok = false;
  }
  else
  {
    status_update("SPIFFS Mount OK");
    spiffs_ok = true;
  }

  // try connecting to the wifi network
  if (use_wifi) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(network_ssid, network_password);
    status_update("Network: ");
    status_update(network_ssid, false);
    for(int i=0; i<20; i++)
    {
      if (WiFi.status() == WL_CONNECTED) {
        break;
      }
      delay(500);
      status_update(".", false);
    }
    if (WiFi.status() != WL_CONNECTED)
      status_update(" --> failed.");
    else
      status_update(" --> connected.");
  }

  // if we're not connected to wifi, start an access point
  if (!use_wifi || (WiFi.status() != WL_CONNECTED)){
    status_update("AP mode:", false);
    WiFi.softAP(ssid);
    IPAddress myIP = WiFi.softAPIP();
    // WiFi.softAPsetHostname(hostname);
    // Serial.print("AP IP address: ");
    // Serial.println(myIP);
    ip_addr_str = myIP.toString();
  } else {
    status_update("Network mode: ", false);
    // Serial.print("IP address: ");
    // Serial.println(WiFi.localIP());
    ip_addr_str = WiFi.localIP().toString();
  }
  status_update(ip_addr_str.c_str());

  // mdns_init();
  // mdns_hostname_set(ssid);

  // dynamic pages
  status_update("Starting server...");
  // server.on("/", handleLandingPage);
  server.on("/move", handleMove);      // immediate move
  server.on("/turn", handleTurn);      // immediate turn
  server.on("/stop", handleStop);      // immediate stop (of everything)
  server.on("/plan", handlePlan);      // run multiple commands (BUCL script)
  server.on("/busy", handleBusy);      // run multiple commands (BUCL script)
  server.on("/luminosity1", handleLuminosity1); // get luminosity
  server.on("/luminosity2", handleLuminosity2); // get luminosity
  server.on("/distance", handleDistance); // get distance
  server.on("/accel-x", handleAccel_x); // accelerometer x axis
  server.on("/accel-y", handleAccel_y); // accelerometer y axis
  server.on("/accel-z", handleAccel_z); // accelerometer z axis
  server.on("/servoGo", handleServoGo); // move servo!
  server.on("/servoInit", handleServoInit); // move servo!
  server.on("/led", handleLED); // led 
  // server.on("/settings", handleSetup);
  server.on("/save", handleSave);
  server.onNotFound(handleNotFound); // generic page handler
  server.begin();


  // set up the motor step timer
  status_update("Starting motors...");
  setup_timer();

  // see what's on the filesystem (and add it to the server)
  status_update("Adding files...");
  addAllFiles();

  // initialize the servos
  status_update("Initializing servos...");
  servoInit();

  // setup complete
  status_update("boxbot ready");
}

void loop()
{
  server.handleClient(); // close out any open/pending web transactions
  executePlan();         // returns immediately if there's no plan, loops there if there is a plan
}
