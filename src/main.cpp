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
#include "util.h"

// setting up lisp runtime
#include "lisp.h"
typedef Lisp<8192,2048> MySmallLisp;

#define SDEBUG(label, x) {Serial.print(label); Serial.println(x);}
#define ABS(x) ((x<0) ? (-x) : (x))

// the gui is (currently) served as static "files" from memory
// #include "html_files.h"  // script-generated in the repl.it repo: https://replit.com/@gever/bbdebuggui
WebServer server(80);

// things that can be set through settings UI
// TODO: store these in flash filesystem
uint16_t motor_step_rate = 950;
float linear_turn_fudge = 1.0;
float linear_motion_fudge = 1.0;
bool wheels_forward = true;

// Set these to your desired credentials.
const char *ssid = "silly-bobcat";
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
#define SET_BITS(a, b, c, d) { DW(pin1, a); DW(pin2, b); DW(pin3, c); DW(pin4, d); }

class MarsStepper {
  private:
    bool enabled;
    bool dir;
    unsigned short pin1, pin2, pin3, pin4;
    unsigned int step_counter;
    unsigned int divisor;
  public:
    unsigned int current_step;
    MarsStepper(int motor_pin_1, int motor_pin_2,
                int motor_pin_3, int motor_pin_4) {
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
      divisor = 1; // how often step() calls should be heeded

      enabled = false;
      dir = FWD;
    }
    void set_direction(bool d) {
      // SDEBUG("set_direction = ", d);
      dir = d;
    }
    void enable() {
      enabled = true;
    }
    void disable() {
      enabled = false;
      SET_BITS(0, 0, 0, 0);
    }
    void set_divisor( unsigned int v ) {
      divisor = v;
    }
    void step() {
      // always increment the counter
      step_counter++;

      // don't do anything if we're not running
      if (!enabled) return;

      // don't do anything if it's not our turn yet
      if (step_counter % divisor)
        return;

      // step! (activate the next set of stators)
      switch ( current_step % 8 ) {
        case 0: SET_BITS(1, 0, 0, 0); break;
        case 1: SET_BITS(1, 1, 0, 0); break;
        case 2: SET_BITS(0, 1, 0, 0); break;
        case 3: SET_BITS(0, 1, 1, 0); break;
        case 4: SET_BITS(0, 0, 1, 0); break;
        case 5: SET_BITS(0, 0, 1, 1); break;
        case 6: SET_BITS(0, 0, 0, 1); break;
        case 7: SET_BITS(1, 0, 0, 1); break;
      }
      current_step += (dir ? 1 : -1); // FWD=true, BWD=false
    }
};

// create the motors
MarsStepper m1(13, 14, 27, 26);
MarsStepper m2(15, 2, 4, 19);

// allocate the sensor pins 
//const int pingPin = 5; // Trigger Pin of Ultrasonic Sensor
//const int echoPin = 18; // Echo Pin of Ultrasonic Sensor
//int servoPin = 32;

const float rot_steps = 2038 * 2;   // one full rotation of the motor shaft
const float boxbot_body_rad = 75;   // the distance from the pen to the wheel in mm
const float boxbot_turn_circ = PI * 2.0 * boxbot_body_rad;  // circumference of circle described by the wheels
const float boxbot_wheel_rad = 40;  // in mm
const float boxbot_wheel_circ = PI * 2.0 * boxbot_wheel_rad;  // circumference of the wheel in mm
const float boxbot_turn_steps = (boxbot_turn_circ / boxbot_wheel_circ) * rot_steps; // number of steps to turn 360 deg
const float boxbot_steps_mm = rot_steps / boxbot_wheel_circ;  // steps per mm

// #define ROT_STEPS  2038*2             // one complete rotation
// #define ROT_STEPSF ((float)ROT_STEPS)
// #define ROT_UNIT (ROT_STEPS/16)

void step_the_motors();
void IRAM_ATTR onTimer() {
  step_the_motors();
}

int step_count = 0; // for the current motion, for all active motors

/*
   set up a move command
   dir  - fwd=1, bwd=0
   dist - in centimeters
*/
void setup_move(bool dir, int dist) {
  // SDEBUG("setup_move\ndir = ", dir);
  // SDEBUG("dist = ", dist);
  step_count = (dist * 10) * boxbot_steps_mm * linear_motion_fudge;
  if (dir) {
    // SDEBUG("FWD:", dir);
    m1.set_direction(0);
    m2.set_direction(1);
  } else {
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
void setup_turn(bool dir, int angle) {
  step_count = boxbot_turn_steps * (((float)angle) / 360.0f) * linear_turn_fudge;
  if (dir) {
    m1.set_direction(1);
    m2.set_direction(1);
  } else {
    m1.set_direction(0);
    m2.set_direction(0);
  }
  m1.enable();
  m2.enable();

  // start movement
  // timerAttachInterrupt(step_timer, &onTimer, true);
}

void step_the_motors() {
  if (step_count) {
    step_count--;

    m1.step();
    m2.step();

    if (step_count == 0) {
      // save power, turn off the motors when not moving
      m1.disable();
      m2.disable();
      // TODO: evaluate possibility of turning off the interrupts while idle (might save power)
    }
  }
}

// configure the interrupt timer
void setup_timer( ) {
  // pretty good tutorial on interrupt timers:
  // https://iotespresso.com/timer-interrupts-with-esp32/
  if (step_timer == NULL) {
    step_timer = timerBegin(0, 80, true);   // configured for 1MHZ (1,000,000/sec)
  }
  timerAttachInterrupt(step_timer, &onTimer, true);
  timerAlarmWrite(step_timer, motor_step_rate, true);
  timerAlarmEnable(step_timer);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}

// CONSIDER: right now you can interrupt the active movement by
//           sending a new movement command

void handleMove() {
  if (server.args()) {
    int v = server.arg(0).toInt();  // negative for backwards movement
    setup_move( v < 0 ? BWD : FWD, ABS(v) );
  }
  server.send(200, "application/json", "{status:'ACK'}");
}

void handleTurn() {
  if (server.args()) {
    int v = server.arg(0).toInt();
    setup_turn( v < 0 ? 0 : 1, ABS(v) ); // negative for left turns
  }
  server.send(200, "application/json", "{status:'ACK'}");
}

void handleStop() {
  m1.disable();
  m2.disable();
  step_count = 0;
  plan_ready = false;
  server.send(200, "application/json", "{status:'ACK'}");
}

void handleSave() {
  // TODO: save current settings to SPIFFS/flash memory
  // TODO: save load current settings from SPIFFS
  if (server.args()) {
    String label;

    label = "motor-step-rate";
    if (server.hasArg( label )) {
      motor_step_rate = server.arg( label ).toInt();
      // TODO: motor_setup( motor_step_rate );
    }
    label = "linear-motion-fudge";
    if (server.hasArg( label )) {
      linear_motion_fudge = server.arg( label ).toFloat();
    }
    label = "linear-turn-fudge";
    if (server.hasArg( label )) {
      linear_turn_fudge = server.arg( label ).toFloat();
    }
  }
  // TODO: handleLandingPage();
}

// parse a string into a number
int parse_int(char *str) {
  int num = 0;
  int i = 0;
  if (str[i] == '-')
    i++;
  while (str[i] >= '0' && str[i] <= '9') {
    num = num * 10 + str[i] - '0';
    i++;
  }
  if (str[0] == '-')
    num *= -1;
  return num;
}

void handlePlan() {
  if (server.args()) {
    if (server.arg(0).length() > MAX_PLAN_LEN-1) {
      Serial.println("ERR: Motion plan length exceeds buffer size - ignoring plan.");
      server.send(200, "application/json", "{status:'ACK'}"); // TODO: send better return statuses
      return;
    }
    
    server.send(200, "application/json", "{status:'ACK'}"); // TODO: send better return statuses

    strcpy(plan_buffer, server.arg(0).c_str());
    plan = plan_buffer;   // point it back at the front of the buffer
    plan_token = strtok(plan, delim); // get the first token
    plan_ready = true;
  }
}

void serveGenericPage(String url) {
  Serial.print("handlePageRequest: " + url + " ");
  if( !SPIFFS.exists(url) ) {
    Serial.println("file does not exist:" + url);
    server.send(404, "text/plain", "file does not exist" + url);
    return;
  }
  
  File file = SPIFFS.open(url, "r");
  if (!file) {
    Serial.println("file open failed" + url);
    server.send(404, "text/plain", "file open failed" + url);
    return;
  }
  String contentType = "text/plain";
  if (url.endsWith(".html")) contentType = "text/html";
  else if (url.endsWith(".js")) contentType = "text/javascript";
  else if (url.endsWith(".css")) contentType = "text/css";
  else if (url.endsWith(".ico")) contentType = "image/x-icon";
  Serial.println(contentType);
  server.streamFile(file, contentType);
  file.close();
}

typedef Lisp<8192,2048> MySmallLisp;
MySmallLisp lisp;
void lispSetup() {
  char buf[4096];
  File f = SPIFFS.open("/init.lisp", "r");
  if (!f) {
    Serial.println("ERR: Failed to open init.lisp");
    return;
  }
  f.readBytes(buf, sizeof(buf));
  f.close();
  lisp.input_from_buffer(buf);
}

void evalCode(const char *code) {
  lisp.unwind();
  lisp.input_from_buffer(code);
  lisp.eval(*lisp.push(lisp.read()), lisp.env);  
  // lisp.print(lisp.eval(*lisp.push(lisp.read()), lisp.env));
}

void handleLisp() {
  // TODO: do this more like handlePlan() above
  if (server.args()) {
    const String label = "code";
    if (server.hasArg( label )) {
      // TODO: configure the lisp input to come from the form field
      String script = server.arg( label );
      evalCode(script.c_str());
    }
  }
  serveGenericPage("lisp_input.html");  // TODO: fill form with previous code
}

void executePlan() {
  // only parse/setup next statement in the plan if we have finished the previous step
  if (plan_ready && (step_count == 0)) {
    int num = 0;
    if (*plan_token == 'M') {
      num = parse_int(++plan_token);
      setup_move(num < 0 ? BWD : FWD, num < 0 ? -num : num);
    } else if (*plan_token == 'T') {
      num = parse_int(++plan_token);
      setup_turn(num < 0 ? 0 : 1, num < 0 ? -num : num); 
    } else if (*plan_token == 'P') {
      num = parse_int(++plan_token);
      // TODO: set_pen_position( num );
    }
    
    plan_token = strtok(NULL, delim);  // set up the next bit of code to execute
    if (plan_token == NULL) { // finished the script
      Serial.println("executePlan: end");
      plan_ready = false;
    }
  }
}

// handle request for static page content from the SPIFFS filesystem
void handlePageRequest() {
  String url = server.uri();
  serveGenericPage(url);
  // server.send(SPIFFS, url, contentType);
}

void addAllFiles() {
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while(file){
    Serial.print("  FILE: ");
    Serial.println(file.name());
    server.on("/" + String(file.name()), handlePageRequest);
    file = root.openNextFile();
  }
}

void setup() {
  bool spiffs_ok = false;

  Serial.begin(115200);
  while(!Serial) { delay(10); } // wait for serial port to connect. Needed for native USB port only

  Serial.println("\n\nBoxbot v0.1 --------");
  if (!SPIFFS.begin(true)) {
    Serial.println("ERR: SPIFFS Mount Failed");
    spiffs_ok = false;
  } else {
    Serial.println("SPIFFS Mount OK");
    spiffs_ok = true;
  }

  Serial.println("Configuring access point...");
  WiFi.softAP(ssid);
  IPAddress myIP = WiFi.softAPIP();
  // WiFi.softAPsetHostname(hostname);
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  Serial.println("Configuring server...");
  // dynamic pages
  // server.on("/", handleLandingPage);
  server.on("/move", handleMove);   // immediate move
  server.on("/turn", handleTurn);   // immediate turn
  server.on("/stop", handleStop);   // immediate stop (of everything)
  server.on("/plan", handlePlan);     // run multiple commands (BUCL script)
  server.on("/lisp_code", handleLisp);   // execute lisp fragment (for testing)
  // server.on("/settings", handleSetup);
  server.on("/save", handleSave);
  server.onNotFound(handleNotFound);

  // initialize lisp
  Serial.println("Configuring lisp...");
  lispSetup();

  Serial.println("Starting server...");
  server.begin();

  // set up the motor step timer
  Serial.println("Starting motors...");
  setup_timer( );

  // see what's on the filesystem (and add it to the server)
  addAllFiles();
}

void loop() {
  // Put sensor test code here, change #if 1 to #if 0 to disable main loop
#if 1
  server.handleClient();  // close out any open/pending web transactions
  executePlan();      // returns immediately if there's no plan, loops there if there is a plan
#endif
  delay(2);            // allow the cpu to switch to other tasks
}
