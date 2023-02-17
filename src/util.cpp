/*
   MarsStepper
   - supports half-stepping multiple 28ByJ-48 motors on a single Arduino
   
   Usage:
   Stepper(D2, D3, D4, D5) - creates a stepper object in the disabled state
   set_direction( FWD or BWD )
   enable()
   disable()
   step() - call step at the rate that supports the speed you want the motors to go
*/
#include <Arduino.h>
#include <ArduinoJson.h>

/*
 * JSON support
 */
StaticJsonDocument<250> jsonDocument;
char buffer[250];
const char* create_json(char *tag, float value) {  
  jsonDocument.clear();
  jsonDocument["type"] = tag;
  jsonDocument["value"] = value;
  serializeJson(jsonDocument, buffer);
  return buffer;  
}
 
void add_json_object(char *tag, float value) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj["type"] = tag;
  obj["value"] = value;
}
