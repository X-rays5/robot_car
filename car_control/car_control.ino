#include <Arduino.h>
#include <Servo.h>
#include <ArduinoJson.h>
#include "drive_defines.hpp"

#define BAUD_RATE 9600
#define ULTRASONIC_PIN 3

Servo ultrasonic_servo;

int GetUltrasonicPos() {
  return ultrasonic_servo.read();
}

void SetUltrasonicPos(int val) {
  ultrasonic_servo.write(val);
}

void IncreaseUltrasonicPos(int increase_by) {
  ultrasonic_servo.write(ultrasonic_servo.read() + increase_by);
}

void DecreaseUltrasonicPos(int decrease_by) {
  ultrasonic_servo.write(ultrasonic_servo.read() - decrease_by);
}

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

  ultrasonic_servo.attach(ULTRASONIC_PIN);
  SetUltrasonicPos(90);

  digitalWrite(13, LOW);
}

unsigned long time_since_update = 0;
unsigned long time_since_ultrasonic_move = 0;
bool ultra_sonic_moving_left = false;
bool ultra_sonic_moving_right = false;
#define ULTRASOUND_LEFT ultra_sonic_moving_left = true; ultra_sonic_moving_right = false
#define ULTRASOUND_RIGHT ultra_sonic_moving_left = false; ultra_sonic_moving_right = true
#define ULTRASOUND_STOP ultra_sonic_moving_left = false; ultra_sonic_moving_right = false
#define ULTRASOUND_RESET ULTRASOUND_STOP; SetUltrasonicPos(90)
void loop() {
  if (millis() - time_since_ultrasonic_move >= 20) {
    if(ultra_sonic_moving_left) {
      IncreaseUltrasonicPos(1);
    } else if(ultra_sonic_moving_right) {
      DecreaseUltrasonicPos(1);
    }
    time_since_ultrasonic_move = millis();
  }

  if(Serial.available() > 0)  {
    digitalWrite(13, HIGH);
    DynamicJsonDocument json(128);
    auto json_error = deserializeJson(json, Serial);
    if (json.overflowed()) {
      StaticJsonDocument<80> error;
        error["error"] = "Received msg was too long";
        serializeJson(error, Serial);
        return;
    }
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
      } else if(json["event"] == "ultra-left") {
        ULTRASOUND_LEFT;
      } else if(json["event"] == "ultra-right") {
        ULTRASOUND_RIGHT;
      } else if (json["event"] == "ultra-stop") {
        ULTRASOUND_STOP;
      } else if(json["event"] == "ultra-reset") {
        ULTRASOUND_RESET;
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
    serializeJson(json, Serial);
    digitalWrite(13, LOW);
  }
  if (millis() - time_since_update >= 500) {
    {
      StaticJsonDocument<128> json;
      json["type"] = "status";
      json["category"] = "ultrasonic";
      json["data"]["pos"] = GetUltrasonicPos();
      serializeJson(json, Serial);
    }
    time_since_update = millis();
  }
}