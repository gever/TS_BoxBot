#include <ArduinoJson.h>
#include <FS.h>
#include "status.h"
#define DEFINE_GLOBALS 1
#include "settings.h"

void set_setting(setting_t *setting, const String& value) {
  switch (setting->type) {
    case INT:
      *setting->int_target = value.toInt();
      break;
    case FLOAT:
      *setting->float_target = value.toFloat();
      break;
    case BOOL:
      *setting->bool_target = value.toInt();
      break;
    case STRING:
      snprintf(setting->string_target, SETTINGS_STR_BUFFER_SIZE, "%s", value.c_str());
      break;
  }
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

// reset the settings to their default values
void reset_settings() {
  status_update("Resetting settings");
  motor_step_rate = RESET_MOTOR_STEP_RATE;
  linear_turn_fudge = RESET_LINEAR_TURN_FUDGE;
  linear_motion_fudge = RESET_LINEAR_MOTION_FUDGE;
  invert_direction = RESET_INVERT_DIRECTION;
  invert_turn = RESET_INVERT_TURN;
  use_wifi = RESET_USE_WIFI;
  strncpy(buffer_ap_ssid, RESET_AP_SSID, SETTINGS_STR_BUFFER_SIZE);
  strncpy(buffer_ap_password, RESET_AP_PASSWORD, SETTINGS_STR_BUFFER_SIZE);
  strncpy(buffer_network_ssid, RESET_NETWORK_SSID, SETTINGS_STR_BUFFER_SIZE);
  strncpy(buffer_network_password, RESET_NETWORK_PASSWORD, SETTINGS_STR_BUFFER_SIZE);
  save_settings();
}
