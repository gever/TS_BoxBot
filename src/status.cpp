#include <Wire.h>
#include "status.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
bool license_plate_detected = false;

bool license_plate_init()
{
    // check to see if the OLED display is at I2C address 0x3C
    Wire.begin();
    Wire.beginTransmission(0x3C);
    if (Wire.endTransmission() == 0)
    {
        Serial.println("OLED found at 0x3C");
        license_plate_detected = true;
    }
    else
    {
        Serial.println("OLED not found at 0x3C");
        license_plate_detected = false;
        return false;
    }

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("SSD1306 allocation failed"));
        return false;
    }
    display.display();
    delay(1000); // Pause for 1 second
    display.clearDisplay();
    display.display();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    return true;
}

// display current status or boot progress
#define TEXT_LINES 6
#define TEXT_LINE_LEN 32
#define TEXT_DISPLAY_TOP 16   // where the text display starts (empirically determined)

// bold yellow messages
bool first_activity = true;
void activity_update(const char *msg) {
  Serial.println(msg);
  if (!license_plate_detected) {
    return;
  }

  if (first_activity) {
    first_activity = false;
    display.clearDisplay();
  } else {
    display.fillRect(0, 0, 128, TEXT_DISPLAY_TOP, BLACK);
  }
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(msg);
  display.setTextSize(1);
  display.display();
}
void activity_update(const char *msg1, const char *msg2) {
  char buf[TEXT_LINE_LEN];
  snprintf(buf, TEXT_LINE_LEN, "%s %s", msg1, msg2);
  activity_update(buf);
}
void activity_update(const char *msg1, const char *msg2, const char *msg3) {
  char buf[TEXT_LINE_LEN];
  snprintf(buf, TEXT_LINE_LEN, "%s %s %s", msg1, msg2, msg3);
  activity_update(buf);
}

// regular status messages
char status_buffer[TEXT_LINES][TEXT_LINE_LEN];
void status_init() {
  memset(status_buffer, 0, sizeof(status_buffer));
}
void status_update(const char *msg)
{
  Serial.println(msg);
  if (!license_plate_detected) {
    return;
  }

  // scroll up, write new message at bottom of screen
  memcpy(status_buffer[0], status_buffer[1], TEXT_LINE_LEN * (TEXT_LINES - 1));
  strncpy(&status_buffer[TEXT_LINES - 1][0], msg, TEXT_LINE_LEN - 1);

  // redraw the blue section of the display
  display.fillRect(0, TEXT_DISPLAY_TOP, 128, 64 - TEXT_DISPLAY_TOP, BLACK);
  display.setCursor(0, TEXT_DISPLAY_TOP);
  for (int i = 0; i < TEXT_LINES; i++) {
    display.println(status_buffer[i]);
  }
  display.display();
}

void status_update(const char *msg1, const char *msg2)
{
  char buf[TEXT_LINE_LEN];
  snprintf(buf, TEXT_LINE_LEN, "%s %s", msg1, msg2);
  status_update(buf);
}

void status_update(const char *msg1, const char *msg2, const char *msg3)
{
  char buf[TEXT_LINE_LEN];
  snprintf(buf, TEXT_LINE_LEN, "%s %s %s", msg1, msg2, msg3);
  status_update(buf);
}
