#pragma once
#include <vector>
#include "Arduino.h"
#include <SPIFFS.h>
#include "global.h"

// things that can be set through settings UI
// TODO: store these in SPIFFS filesystem
#define SETTINGS_STR_BUFFER_SIZE 64
#define RESET_MOTOR_STEP_RATE   950
#define RESET_LINEAR_TURN_FUDGE 1.0
#define RESET_LINEAR_MOTION_FUDGE 1.0
#define RESET_INVERT_DIRECTION  false
#define RESET_INVERT_TURN       false
#define RESET_USE_WIFI          false
#define RESET_AP_SSID           "boxbot*" // * is replaced with the identifier generated from the MAC address
#define RESET_AP_PASSWORD       ""
#define RESET_NETWORK_SSID      "your_ssid_here"
#define RESET_NETWORK_PASSWORD  "your_password_here"

// Note: in c++17 this can be done by just using an 'inline' declaration.
GLOBAL(uint16_t, motor_step_rate, RESET_MOTOR_STEP_RATE);
GLOBAL(float, linear_turn_fudge, RESET_LINEAR_TURN_FUDGE);
GLOBAL(float, linear_motion_fudge, RESET_LINEAR_MOTION_FUDGE);
GLOBAL(bool, invert_direction, RESET_INVERT_DIRECTION);
GLOBAL(bool, invert_turn, RESET_INVERT_TURN);
GLOBAL(bool, use_wifi, RESET_USE_WIFI);
GLOBAL(char, buffer_ap_ssid[SETTINGS_STR_BUFFER_SIZE], RESET_AP_SSID);
GLOBAL(char, buffer_ap_password[SETTINGS_STR_BUFFER_SIZE], RESET_AP_PASSWORD);
GLOBAL(char, buffer_network_ssid[SETTINGS_STR_BUFFER_SIZE], RESET_NETWORK_SSID);
GLOBAL(char,  buffer_network_password[SETTINGS_STR_BUFFER_SIZE], RESET_NETWORK_PASSWORD);

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

GLOBAL_EXTERN setting_t settings[10]
#ifdef DEFINE_GLOBALS
= {
  {"use-wifi",            { .bool_target   = &use_wifi},               BOOL},
  {"ap-ssid",             { .string_target = buffer_ap_ssid},          STRING},
  {"ap-password",         { .string_target = buffer_ap_password},      STRING},
  {"network-ssid",        { .string_target = buffer_network_ssid},     STRING},
  {"network-password",    { .string_target = buffer_network_password}, STRING},
  {"motor-step-rate",     { .int_target    = &motor_step_rate},        INT},
  {"linear-turn-fudge",   { .float_target  = &linear_turn_fudge},      FLOAT},
  {"linear-motion-fudge", { .float_target  = &linear_motion_fudge},    FLOAT},
  {"invert-direction",    { .bool_target   = &invert_direction},       BOOL},
  {"invert-turn",         { .bool_target   = &invert_turn},            BOOL}
}
#endif
;

// set a setting to a value from a String
void set_setting(setting_t *setting, const String &value);

// save the settings to SPIFFS as JSON
void save_settings();

// load the settings from SPIFFS
void load_settings();

// render the current settings as lines of text
String render_settings();

// reset the settings to their default values
void reset_settings();
