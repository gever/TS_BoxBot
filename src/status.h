#pragma once
// Display includes and defs
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define TEXT_LINE_LEN 32


bool license_plate_init();

inline void message_builder(char buf[TEXT_LINE_LEN], int len) {}

template<typename T, typename... Args>
void message_builder(char buf[TEXT_LINE_LEN], int len, T arg, Args... args) {
  len -= snprintf(buf + TEXT_LINE_LEN - len, len, " %s", arg);
  message_builder(buf, len, args...);
}

// activity messages go to the top line of the display
void activity_update_message(const char *msg);
template <typename... Args>
void activity_update(const char* msg, Args... args) {
  char buf[TEXT_LINE_LEN];
  int len = TEXT_LINE_LEN;

  len -= snprintf(buf, len, "%s", msg);
  message_builder(buf, len, args...);
  activity_update_message(buf);
}

// status messages scroll in the lower section of the display
void status_update_message(const char *msg);
template <typename... Args>
void status_update(const char* msg, Args... args) {
  char buf[TEXT_LINE_LEN];
  int len = TEXT_LINE_LEN;

  len -= snprintf(buf, len, "%s", msg);
  message_builder(buf, len, args...);
  status_update_message(buf);
}
