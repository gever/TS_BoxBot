// things that can be set through settings UI
// TODO: store these in SPIFFS filesystem
#define SETTINGS_STR_BUFFER_SIZE 64
#define RESET_MOTOR_STEP_RATE   950
#define RESET_LINEAR_TURN_FUDGE 1.0
#define RESET_LINEAR_MOTION_FUDGE 1.0
#define RESET_WHEELS_FORWARD    true
#define RESET_USE_WIFI          false
#define RESET_AP_SSID           "wacky-squirrel"
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
  {"motor-step-rate",     .int_target   = &motor_step_rate,     .type = INT},
  {"linear-turn-fudge",   .float_target = &linear_turn_fudge,   .type = FLOAT},
  {"linear-motion-fudge", .float_target = &linear_motion_fudge, .type = FLOAT},
  {"wheels-forward",      .bool_target  = &wheels_forward,      .type = BOOL},
  {"use-wifi",            .bool_target  = &use_wifi,            .type = BOOL},
  {"ap-ssid",            .string_target = buffer_ap_ssid,       .type = STRING},
  {"ap-password",        .string_target = buffer_ap_password,   .type = STRING},
  {"network-ssid",       .string_target = buffer_network_ssid,  .type = STRING},
  {"network-password",   .string_target = buffer_network_password, .type = STRING}
};

// save the settings to SPIFFS as CSV by label, value
void save_settings() {
  File file = SPIFFS.open("/settings.csv", "w");
  if (!file) {
    Serial.println("ERR: failed to open settings file for writing");
    return;
  }
  for (int i = 0; i < sizeof(settings) / sizeof(setting_t); i++) {
    switch (settings[i].type) {
      case INT:
        file.print(settings[i].label);
        file.print(",");
        file.println(*settings[i].int_target);
        break;
      case FLOAT:
        file.print(settings[i].label);
        file.print(",");
        file.println(*settings[i].float_target);
        break;
      case BOOL:
        file.print(settings[i].label);
        file.print(",");
        file.println(*settings[i].bool_target);
        break;
      case STRING:
        file.print(settings[i].label);
        file.print(",");
        file.println(settings[i].string_target);
        break;
    }
  }
  file.close();
}

// load the settings from SPIFFS as CSV by label, value
void load_settings() {
  File file = SPIFFS.open("/settings.csv", "r");
  if (!file) {
    Serial.println("ERR: failed to open settings file for reading");
    status_update("ERR: failed to open settings file for reading");
    return;
  }
  char line[128];   // TODO: check for buffer overflow
  while (file.available()) {
    int len = file.readBytesUntil('\n', line, 128);
    // TODO: fix bug where we still get garbage characters at the end of the STRING values
    line[len] = 0; // null terminate the string, because readBytesUntil doesn't
    // ignore lines that start with a comment
    if (line[0] == '#') {
      continue;
    }
    char *label = strtok(line, ",");
    char *value = strtok(NULL, "\n");
    // find the matching label in the settings table
    for (int i = 0; i < sizeof(settings) / sizeof(setting_t); i++) {
      if (strcmp(label, settings[i].label) == 0) {
        switch (settings[i].type) {
          case INT:
            *settings[i].int_target = atoi(value);
            break;
          case FLOAT:
            *settings[i].float_target = atof(value);
            break;
          case BOOL:
            *settings[i].bool_target = atoi(value);
            break;
          case STRING:
            memset(settings[i].string_target, 0, SETTINGS_STR_BUFFER_SIZE);
            strcpy(settings[i].string_target, value);
            break;
        }
      }
    }
  }
  file.close();
  status_update("settings loaded");
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
        text += settings[i].string_target;
        text += "\n";
        break;
    }
  }
  return text;
}
