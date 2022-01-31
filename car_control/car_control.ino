//#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include "drive_defines.hpp"

#define BAUD_RATE 9600

void setup() {
  Serial.begin(BAUD_RATE);
  pinMode(13, OUTPUT);

  // setup motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
}

void loop() {
  if(Serial.available() > 0)  {
    String data = Serial.readString();
    DynamicJsonDocument doc(512);
    auto json_error = deserializeJson(doc, data);
    auto json = doc.as<JsonObject>();
    if (!json_error) {
      if(json["event"] == "forward") {
        FORWARD;
      } else if(json["event"] == "back") {
        BACK;
      } else if(json["event"] == "left") {
        LEFT;
      } else if(json["event"] == "right") {
        RIGHT;
      } else if(json["event"] == "stop") {
        STOP;
      } else {
        DynamicJsonDocument json(256);
        doc["error"] = "Error invalid event";
        serializeJson(doc, Serial);
      }
    } else {
      DynamicJsonDocument json(256);
      doc["error"] = "Error parsing json";
      serializeJson(doc, Serial);
    }
  }                            
}
