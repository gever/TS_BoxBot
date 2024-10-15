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
#include <DNSServer.h>
// #include <mdns.h>

#include "status.h"
#include "util.h"
#include "sensors.h"
#include "servo.h"
#include "led.h"


//
// canonical version number
// 0.6: sent to PVUSD on 2021-04-01
// 0.7: establish canonical version number, add DNS server, and version api
// 0.71: fix forward/backward inversion settings
// 0.72b: young robotics week version
// 0.73: separate out status code, change default direction of motors 
#define VERSION "0.73"

#define SDEBUG(label, x) \
  {                      \
    Serial.print(label); \
    Serial.println(x);   \
  }
#define ABS(x) ((x < 0) ? (-x) : (x))

WebServer server(80);

// add the variables to the settings table
#include "settings.h"

// primitive motion plan parsing/interpreting
#define MAX_PLAN_LEN 1024
char *plan_token = NULL;
bool plan_ready = false;
char plan_buffer[MAX_PLAN_LEN];
char *plan = NULL;
const char delim[] = ",\n";

// hardware clock to step the motors at a good rate
hw_timer_t *step_timer = NULL;

// add dns server
const byte DNS_PORT = 53;
DNSServer dnsServer;

// domain name for the DNS server, can't use .local conflicts with multicast DNS (mDNS) or Bonjour services
const char *dnsDomain = "boxbot.home";  

const bool FWD = true;  // forward in terms of the boxbot, not motor
const bool BWD = false; // backward in terms of the boxbot, not motor
const bool LFT = true;  // left in terms of the boxbot, not motor
const bool RGT = false; // right in terms of the boxbot, not motor
#define CW 1
#define CCW 0
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
    dir = CW;
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
    current_step += (dir ? 1 : -1); // CW=true, CCW=false
  }
};

// create the motors
MarsStepper motor_rt(13, 14, 27, 26); // right
MarsStepper motor_lt(15, 2, 4, 19);   // left

const float rot_steps = 2038 * 2;                                                   // one full rotation of the motor shaft
const float boxbot_body_rad = 125/2.0f;                                             // the distance from the pen to the wheel in mm
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
  motor_lt.disable();
  motor_rt.disable();
}


/*
   set up a move command
   dir  - fwd=1, bwd=0
   dist - in centimeters
*/
void setup_move(boolean dir, int dist)
{
  // SDEBUG("setup_move\ndir = ", dir);
  // SDEBUG("dist = ", dist);
  if (invert_direction) dir = !dir;
  step_count = (dist * 10) * boxbot_steps_mm * linear_motion_fudge;
  if (dir)
  {
    // SDEBUG("FWD:", dir);
    motor_rt.set_direction(CCW);
    motor_lt.set_direction(CW);
  }
  else
  {
    // SDEBUG("BWD:", dir);
    motor_rt.set_direction(CW);
    motor_lt.set_direction(CCW);
  }
  motor_rt.enable();
  motor_lt.enable();

  // start movement
  // timerAttachInterrupt(step_timer, &onTimer, true);
}

/*
   set up a turn command
*/
void setup_turn(bool dir, int angle)
{
  if (invert_turn) dir = !dir;
  step_count = boxbot_turn_steps * (((float)angle) / 360.0f) * linear_turn_fudge;
  if (dir)
  {
    motor_rt.set_direction(CCW);
    motor_lt.set_direction(CCW);
  }
  else
  {
    motor_rt.set_direction(CW);
    motor_lt.set_direction(CW);
  }
  motor_rt.enable();
  motor_lt.enable();

  // start movement
  // timerAttachInterrupt(step_timer, &onTimer, true);
}

void step_the_motors()
{
  if (step_count)
  {
    step_count--;

    motor_rt.step();
    motor_lt.step();

    if (step_count == 0)
    {
      // save power, turn off the motors when not moving
      motor_rt.disable();
      motor_lt.disable();
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
  unsigned long startTime = millis();
  unsigned long setupTime = 0;

  if (server.args()) {
    int v = server.arg(0).toInt(); // negative for backwards movement
    unsigned long cmdStartTime = millis();
    setup_move(v < 0 ? BWD : FWD, ABS(v));
    unsigned long cmdEndTime = millis();
    setupTime = cmdEndTime - cmdStartTime;

    if (v < 0) {
      activity_update("BWD", String(-v).c_str());
    } else {
      activity_update("FWD", String(v).c_str());
    }
  } else {
    server.send(400, "application/json", "{\"status\":\"Error: No arguments provided\"}");
    return;
  }

  server.send(200, "application/json", "{\"status\":\"ACK\"}");

  unsigned long endTime = millis();
  Serial.print("Total handleMove execution time: ");
  Serial.print(endTime - startTime);
  Serial.println(" ms");
  Serial.print("setup_move execution time: ");
  Serial.print(setupTime);
  Serial.println(" ms");
}

void handleTurn()
{
  unsigned long startTime = millis();
  unsigned long setupTime = 0;

  if (server.args())
  {
    int v = server.arg(0).toInt();
    unsigned long cmdStartTime = millis();
    setup_turn(v < 0 ? 1 : 0, ABS(v)); // negative for left turns
    unsigned long cmdEndTime = millis();
    setupTime = cmdEndTime - cmdStartTime;

    if (v < 0) {
      activity_update("LFT", String(-v).c_str());
    } else {
      activity_update("RGT", String(v).c_str());
    }
  } else {
    server.send(400, "application/json", "{\"status\":\"Error: No arguments provided\"}");
    return;
  }
  server.send(200, "application/json", "{\"status\":\"ACK\"}");

  unsigned long endTime = millis();
  Serial.print("Total handleTurn execution time: ");
  Serial.print(endTime - startTime);
  Serial.println(" ms");
  Serial.print("setup_turn execution time: ");
  Serial.print(setupTime);
  Serial.println(" ms");
}

void handleStop()
{
  motor_rt.disable();
  motor_lt.disable();
  step_count = 0;
  plan_ready = false;
  activity_update("STOP");
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
  activity_update("LUM1", String(v).c_str());
  server.send(200, "application/json", jsonBuffer);
}

void handleLuminosity2()
{
  char jsonBuffer[JSON_BUFFER_SIZE];
  int v = getLuminosity2();
  snprintf(jsonBuffer, JSON_BUFFER_SIZE, "{\"luminosity2\":%d}", v);
  activity_update("LUM2", String(v).c_str());
  server.send(200, "application/json", jsonBuffer);
}


void handleDistance()
{
  char jsonBuffer[JSON_BUFFER_SIZE];
  int v = getDistance();
  Serial.print(v);
  snprintf(jsonBuffer, JSON_BUFFER_SIZE, "{\"distance\":%d}", v);
  activity_update("DIST", String(v).c_str());
  server.send(200, "application/json", jsonBuffer);
}

void handleAccel_x()
{
  char jsonBuffer[JSON_BUFFER_SIZE];
  /*
  float v = getAccel_x();
  snprintf(jsonBuffer, JSON_BUFFER_SIZE, "{\"accel x\":%f}", v);
  activity_update("ACCX", String(v).c_str());
  */
  // put a not-supported message in the buffer
  snprintf(jsonBuffer, JSON_BUFFER_SIZE, "{\"accel x\":\"not supported\"}");
  server.send(200, "application/json", jsonBuffer);
}

void handleAccel_y()
{
  char jsonBuffer[JSON_BUFFER_SIZE];
  /*
  float v = getAccel_y();
  snprintf(jsonBuffer, JSON_BUFFER_SIZE, "{\"accel y\":%d}", v);
  activity_update("ACCY", String(v).c_str());
  */
  // put a not-supported message in the buffer
  snprintf(jsonBuffer, JSON_BUFFER_SIZE, "{\"accel y\":\"not supported\"}");
  server.send(200, "application/json", jsonBuffer);
}

void handleAccel_z()
{
  char jsonBuffer[JSON_BUFFER_SIZE];
  /*
  float v = getAccel_z();
  snprintf(jsonBuffer, JSON_BUFFER_SIZE, "{\"accel z\":%d}", v);
  activity_update("ACCZ", String(v).c_str());
  */
  // put a not-supported message in the buffer
  snprintf(jsonBuffer, JSON_BUFFER_SIZE, "{\"accel z\":\"not supported\"}");
  server.send(200, "application/json", jsonBuffer);
}


void handleServoGo()
{
  if (server.args())
  {
    int servoID = server.arg(0).toInt(); // pin
    int servoAngle = server.arg(1).toInt(); // angle
    servoGo(servoID, servoAngle);
    switch(servoID) {
      case 1:
        activity_update("SRVO1", String(servoAngle).c_str());
        break;
      case 2:
        activity_update("SRVO2", String(servoAngle).c_str());
        break;
      case 3:
        activity_update("SRVO3", String(servoAngle).c_str());
        break;
      case 4:
        activity_update("SRVO4", String(servoAngle).c_str());
        break;
    }
  }
  server.send(200, "application/json", "{\"status\":\"ACK\"}");
}

// TODO: deprecate
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
    activity_update("LED", String(pin).c_str(), String(status).c_str());
  }
  server.send(200, "application/json", "{\"status\":\"ACK\"}"); 
}

void handleDetectLine()
{
  char jsonBuffer[JSON_BUFFER_SIZE];
  bool v = detectLine(4095);  // 4095 is for a dark black line on white background
  snprintf(jsonBuffer, JSON_BUFFER_SIZE, "{\"line?\":%s}", v);
  activity_update("LINE", v ? "true" : "false");
  server.send(200, "application/json", jsonBuffer);
}

void handleTemperature()
{
  char jsonBuffer[JSON_BUFFER_SIZE];
  int v = getTemperature();
  snprintf(jsonBuffer, JSON_BUFFER_SIZE, "{\"temperature\":%d}", v);
  activity_update("TEMP", String(v).c_str());
  server.send(200, "application/json", jsonBuffer);
}

void handleGetVars() {
  String html = "<html><head><title>Settings</title></head><body>";
  // add a text input field for the label
  html += "<pre>\n";
  html += render_settings();
  html += "</pre>\n";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleSetVar()
{
  // TODO: save current settings to SPIFFS/flash memory
  // TODO: save load current settings from SPIFFS
  if (server.args())
  {
    if (server.argName(0) == "label") {
      String label;
      String value;

      label = server.arg(0);
      value = server.arg(1);

      Serial.println("handleSetVar: " + label + " " + value);

      // find this var in settings table
      for (int i = 0; i < sizeof(settings) / sizeof(setting_t); i++)
      {
        if (label == settings[i].label)
        {
          switch (settings[i].type)
          {
          case INT:
            *settings[i].int_target = value.toInt();
            break;
          case FLOAT:
            *settings[i].float_target = value.toFloat();
            break;
          case BOOL:
            *settings[i].bool_target = value.toInt();
            break;
          case STRING:
            memset(settings[i].string_target, 0, SETTINGS_STR_BUFFER_SIZE);
            strcpy(settings[i].string_target, value.c_str());
            break;
          }
          status_update("setvar", label.c_str(), value.c_str());
          break;
        }
      }
    } else {
      status_update("ERR: no label for setvar");
    }
    save_settings();
  }
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
    activity_update("PLAN", "loaded");
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
    // Serial.print("  FILE: ");
    // Serial.println(file.name());
    server.on("/" + String(file.name()), handlePageRequest);
    file = root.openNextFile();
  }
  // tack on the default page here
  server.on("/", handlePageRequest);
}

// version API - plain text
void handleVersion()
{
  server.send(200, "text/plain", VERSION);
}

// we always fall back to AP mode if we can't connect to the network
bool network_ap_mode = true;

void setup()
{
  String ip_addr_str = "<not set>";
  bool spiffs_ok = false;
  uint8_t mac[8];

  Serial.begin(115200);
  while (!Serial)
  {
    delay(10);
  } // wait for serial port to connect. Needed for native USB port only
  Serial.println("starting boxbot");
  license_plate_init();
  activity_update("Hello!");
	status_update("boxbot - start");

  // get the MAC address
  esp_read_mac(mac, ESP_MAC_WIFI_STA);

  // rtc.setTime(30,15,23,2,3,2023); // setup the time (this is for the sensors)
  // Serial.println("\n\nBoxbot v0.6 --------");

  if (!SPIFFS.begin(true)) {
    status_update("ERR: SPIFFS mount failed");
    spiffs_ok = false;
  } else {
    status_update("SPIFFS mounted");
    spiffs_ok = true;
  }

  // load settings from SPIFFS before starting the network and server
  if (spiffs_ok) {
    // check to see if the D12 pin is grounded (before we set anything else up)
    // if it is, we'll reset the settings to default
    pinMode(12, INPUT_PULLUP);
    sleep(1);
    if (digitalRead(12) == LOW) {
      status_update("factory reset");
      reset_settings();
    }
    load_settings();
  }

  // check the buffer_ap_ssid and replace the * with the last two bytes of the MAC address
  for (int i = 0; i < strlen(buffer_ap_ssid); i++)
  {
    if (buffer_ap_ssid[i] == '*')
    {
      char temp_ap_ssid[32];
      buffer_ap_ssid[i] = 0;  // null terminate the string at the '*'
      sprintf(temp_ap_ssid, "%s%02X%02X", buffer_ap_ssid, mac[4] ^ mac[5], mac[2] ^ mac[3]);
      strcpy(buffer_ap_ssid, temp_ap_ssid);
      break;
    }
  }

  // try connecting to the wifi network
  if (use_wifi) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(buffer_network_ssid, buffer_network_password);
    status_update("Connecting to Network: ");
    status_update(buffer_network_ssid);
    for(int i=0; i<20; i++) { // wait up to 10 seconds for wifi to connect
      if (WiFi.status() == WL_CONNECTED) {
        break;
      }
      // status_update(".");
      delay(500);
    }
    if (WiFi.status() == WL_CONNECTED) {
      status_update(" --> yay!");
      network_ap_mode = false;
    } else {
      network_ap_mode = true;
      switch(WiFi.status()) {
        case WL_NO_SSID_AVAIL:
          status_update("ERR: no SSID available");
          break;
        case WL_CONNECT_FAILED:
          status_update("ERR: connection failed");
          break;
        case WL_IDLE_STATUS:
          status_update("ERR: idle status");
          break;
        case WL_DISCONNECTED:
          status_update("ERR: disconnected");
          break;
        case WL_NO_SHIELD:
          status_update("ERR: no shield");
          break;
        default:
          status_update("ERR: unknown error");
          break;
      }
    }
  }

  // if we're not connected to wifi, start an access point
  if (!use_wifi || (WiFi.status() != WL_CONNECTED)) {
    status_update("Starting wifi: ", buffer_ap_ssid);
    WiFi.softAP(buffer_ap_ssid);
    IPAddress myIP = WiFi.softAPIP();
    // WiFi.softAPsetHostname(hostname);
    ip_addr_str = myIP.toString();
    network_ap_mode = true;

    String ipStr = "AP IP: " + myIP.toString();
    status_update(ipStr.c_str());

    // start dnsServer
    // dnsServer.start(DNS_PORT, dnsDomain, myIP);
    // status_update("DNS Server Started");
    // status_update("local DN: ", dnsDomain);

  } else {
    ip_addr_str = WiFi.localIP().toString();
  }

  // mdns_init();
  // mdns_hostname_set(ssid);



  // dynamic pages
  status_update("Starting server");
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
  server.on("/setvar", handleSetVar);
  server.on("/getvars", handleGetVars);
  server.on("/version", handleVersion);
  server.onNotFound(handleNotFound); // generic page handler
  server.begin();

  // set up the motor step timer
  status_update("Initialize motors");
  setup_timer();

  // see what's on the filesystem (and add it to the server)
  status_update("Initialize files");
  addAllFiles();

  // initialize the servos
  status_update("Initialize servos");
  servoInit();

  // warm up the sensors (that need it)
  status_update("Initialize sensors");
  // sensors are initialized on first-use
  // sensors_setup();

  // share network info as last thing on the display
  // (format for two lines so it doesn't get cut off)
  if (network_ap_mode) {
    status_update("AP mode:");
    // status_update(buffer_ap_ssid);
    activity_update(buffer_ap_ssid);
  } else {
    status_update("Connected to:");
    status_update("  ", buffer_network_ssid);
  }
  status_update(ip_addr_str.c_str());

  // setup complete
  status_update("boxbot", VERSION, "ready");
}


int last_value = 0;

void loop()
{
  // dnsServer.processNextRequest();  // Handle DNS requests
  server.handleClient(); // close out any open/pending web transactions
  executePlan();         // returns immediately if there's no plan, loops there if there is a plan
}
