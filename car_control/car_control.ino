#include <Arduino.h>
#include <ArduinoJson.h>
#include "drive_defines.hpp"

#define BAUD_RATE 9600

void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  Serial.begin(BAUD_RATE);

  // setup motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  digitalWrite(13, LOW);
}

StaticJsonDocument<256> json;
void loop() {
  if(Serial.available() > 0)  {
    digitalWrite(13, HIGH);
    auto json_error = deserializeJson(json, Serial);
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
        StaticJsonDocument<80> error;
        error["error"] = "Unknown event type";
        serializeJson(error, Serial);
      }
    } else {
      StaticJsonDocument<80> error;
        error["error"] = "Missing event type";
        serializeJson(error, Serial);
    }
    json.clear();
    digitalWrite(13, LOW);
  } else { // No incoming data time to write status update

  }                     
}