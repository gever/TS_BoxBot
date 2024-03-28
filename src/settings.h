#include <ArduinoJson.h>
#include <FS.h>

// things that can be set through settings UI
// TODO: store these in SPIFFS filesystem
#define SETTINGS_STR_BUFFER_SIZE 64
#define RESET_MOTOR_STEP_RATE   950
#define RESET_LINEAR_TURN_FUDGE 1.0
#define RESET_LINEAR_MOTION_FUDGE 1.0
#define RESET_WHEELS_FORWARD    true
#define RESET_USE_WIFI          false
#define RESET_AP_SSID           "boxbot-*" // * is replaced with the last 2 bytes of the MAC address
#define RESET_AP_PASSWORD       ""
#define RESET_NETWORK_SSID      "your_ssid_here"
#define RESET_NETWORK_PASSWORD  "your_password_here"
uint16_t motor_step_rate         = RESET_MOTOR_STEP_RATE;
float linear_turn_fudge          = RESET_LINEAR_TURN_FUDGE;
float linear_motion_fudge        = RESET_LINEAR_MOTION_FUDGE;
bool wheels_forward              = RESET_WHEELS_FORWARD;
bool use_wifi                    = RESET_USE_WIFI;
char buffer_ap_ssid[SETTINGS_STR_BUFFER_SIZE]          = RESET_AP_SSID;
char buffer_ap_password[SETTINGS_STR_BUFFER_SIZE]      = RESET_AP_PASSWORD;
char buffer_network_ssid[SETTINGS_STR_BUFFER_SIZE]     = RESET_NETWORK_SSID;
char buffer_network_password[SETTINGS_STR_BUFFER_SIZE] = RESET_NETWORK_PASSWORD;

enum var_type { INT, FLOAT, BOOL, STRING };
typedef struct {
  const char *label;
  union {
    uint16_t *int_target;
    float *float_target;
    bool *bool_target;
    char *string_target;
  };
  var_type type;
} setting_t;

setting_t settings[] = {
  {"use-wifi",            .bool_target  = &use_wifi,            .type = BOOL},
  {"ap-ssid",            .string_target = buffer_ap_ssid,       .type = STRING},
  {"ap-password",        .string_target = buffer_ap_password,   .type = STRING},
  {"network-ssid",       .string_target = buffer_network_ssid,  .type = STRING},
  {"network-password",   .string_target = buffer_network_password, .type = STRING},
  {"motor-step-rate",     .int_target   = &motor_step_rate,     .type = INT},
  {"linear-turn-fudge",   .float_target = &linear_turn_fudge,   .type = FLOAT},
  {"linear-motion-fudge", .float_target = &linear_motion_fudge, .type = FLOAT},
  {"wheels-forward",      .bool_target  = &wheels_forward,      .type = BOOL}
};

// reset the settings to their default values
void reset_settings() {
  motor_step_rate = RESET_MOTOR_STEP_RATE;
  linear_turn_fudge = RESET_LINEAR_TURN_FUDGE;
  linear_motion_fudge = RESET_LINEAR_MOTION_FUDGE;
  wheels_forward = RESET_WHEELS_FORWARD;
  use_wifi = RESET_USE_WIFI;
  strncpy(buffer_ap_ssid, RESET_AP_SSID, SETTINGS_STR_BUFFER_SIZE);
  strncpy(buffer_ap_password, RESET_AP_PASSWORD, SETTINGS_STR_BUFFER_SIZE);
  strncpy(buffer_network_ssid, RESET_NETWORK_SSID, SETTINGS_STR_BUFFER_SIZE);
  strncpy(buffer_network_password, RESET_NETWORK_PASSWORD, SETTINGS_STR_BUFFER_SIZE);
}
// save the settings to SPIFFS as JSON
void save_settings() {
  File file = SPIFFS.open("/settings.json", "w");
  if (!file) {
    Serial.println("ERR: failed to open settings file for writing");
    return;
  }
  file.print("{");
  for (int i = 0; i < sizeof(settings) / sizeof(setting_t); i++) {
    file.print("\"");
    file.print(settings[i].label);
    file.print("\":");
    switch (settings[i].type) {
      case INT:
        file.print(*settings[i].int_target);
        break;
      case FLOAT:
        file.print(*settings[i].float_target);
        break;
      case BOOL:
        file.print(*settings[i].bool_target);
        break;
      case STRING:
        file.print("\"");
        file.print(settings[i].string_target);
        file.print("\"");
        break;
    }
    if (i < sizeof(settings) / sizeof(setting_t) - 1) {
      file.print(",");
    }
  }
  file.print("}");
  file.close();
}

// load the settings from SPIFFS
void load_settings() {
  File file = SPIFFS.open("/settings.json", "r");
  if (!file) {
    Serial.println("ERR: failed to open settings file for reading");
    return;
  }
  String json = file.readString();
  file.close();
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.println("ERR: failed to parse settings JSON");
    return;
  }
  for (int i = 0; i < sizeof(settings) / sizeof(setting_t); i++) {
    if (doc.containsKey(settings[i].label)) {
      switch (settings[i].type) {
        case INT:
          *settings[i].int_target = doc[settings[i].label];
          break;
        case FLOAT:
          *settings[i].float_target = doc[settings[i].label];
          break;
        case BOOL:
          *settings[i].bool_target = doc[settings[i].label];
          break;
        case STRING:
          strncpy(settings[i].string_target, doc[settings[i].label], SETTINGS_STR_BUFFER_SIZE);
          break;
      }
    }
  }
}

// render the current settings as lines of text
String render_settings() {
  String text = "";
  for (int i = 0; i < sizeof(settings) / sizeof(setting_t); i++) {
    switch (settings[i].type) {
      case INT:
        text += settings[i].label;
        text += " = ";
        text += String(*settings[i].int_target);
        text += "\n";
        break;
      case FLOAT:
        text += settings[i].label;
        text += " = ";
        text += String(*settings[i].float_target);
        text += "\n";
        break;
      case BOOL:
        text += settings[i].label;
        text += " = ";
        text += String(*settings[i].bool_target);
        text += "\n";
        break;
      case STRING:
        text += settings[i].label;
        text += " = ";
        // if label contains 'password' print asterisks
        if (strstr(settings[i].label, "password")) {
          for (int j = 0; j < strlen(settings[i].string_target); j++) {
            text += "*";
          }
        } else {
          text += settings[i].string_target;
        }
        text += "\n";
        break;
    }
  }
  return text;
}
