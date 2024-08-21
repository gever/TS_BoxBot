#ifndef STATUS_H
#define STATUS_H
// Display includes and defs
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64


bool license_plate_init();

// activity messages go to the top line of the display
void activity_update(const char *msg);
void activity_update(const char *msg1, const char *msg2);
void activity_update(const char *msg1, const char *msg2, const char *msg3);

// status messages scroll in the lower section of the display
void status_update(const char *msg);
void status_update(const char *msg1, const char *msg2);
void status_update(const char *msg1, const char *msg2, const char *msg3);

#endif // STATUS_H