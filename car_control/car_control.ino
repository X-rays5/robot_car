// Since ArduinoBle won't work because the mtu is locked at 23 bytes
// web bluetooth denies requests larger then the max payload instead of splitting them up
// so incoming messages will always need to be 20 bytes max
// but we are able to send more then 20 bytes succesfully which will cant then be combined again by the connected client
// so incoming messages will be short plain text messages but outgoing messages will be JSON

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

  if (Serial.available()) {
    digitalWrite(13, HIGH);
    String msg;
    while (Serial.available()) {
      char c = Serial.read();
      if (c == ';') {
        msg.concat(c);
        break;
      } else {
        msg.concat(c);
      }
      delay(5);
    }
    if (msg[msg.length()-1] == ';') {
      msg.remove(msg.length()-1, 1);
      if(msg == "forward") {
        FORWARD;
      } else if(msg == "back") {
        BACK;
      } else if(msg == "left") {
        LEFT;
      } else if(msg == "right") {
        RIGHT;
      } else if(msg == "stop") {
        STOP;
      } else if(msg == "ultra-left") {
        ULTRASOUND_LEFT;
      } else if(msg == "ultra-right") {
        ULTRASOUND_RIGHT;
      } else if (msg == "ultra-stop") {
        ULTRASOUND_STOP;
      } else if(msg == "ultra-reset") {
        ULTRASOUND_RESET;
      } else {
        StaticJsonDocument<80> error;
        error["error"] = "Unknown event type";
        serializeJson(error, Serial);
      }
    }
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