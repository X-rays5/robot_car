// Since ArduinoBle won't work because the mtu is locked at 23 bytes
// web bluetooth denies requests larger then the max payload instead of splitting them up
// so incoming messages will always need to be 20 bytes max
// but we are able to send more then 20 bytes succesfully which will cant then be combined again by the connected client
// so incoming messages will be short plain text messages but outgoing messages will be JSON

#include <Arduino.h>
#include <Servo.h>
#include <ArduinoJson.h>
#include <IRremote.h>
#include "drive_defines.hpp"

#define BAUD_RATE 9600
#define ULTRASONIC_SERVO_PIN 3
#define ULTRASONIC_PIN_ECHO A4
#define ULTRASONIC_PIN_TRIGGER A5
#define ULTRASONIC_LEFT ultra_sonic_moving_left = true; ultra_sonic_moving_right = false
#define ULTRASONIC_RIGHT ultra_sonic_moving_left = false; ultra_sonic_moving_right = true
#define ULTRASONIC_STOP ultra_sonic_moving_left = false; ultra_sonic_moving_right = false
#define ULTRASONIC_RESET ULTRASONIC_STOP; SetUltrasonicPos(90)
#define LINE_TRACKING_1 digitalRead(10)
#define LINE_TRACKING_2 digitalRead(4)
#define LINE_TRACKING_3 digitalRead(2)
#define IR_PIN 12

Servo ultrasonic_servo;
unsigned long time_since_ultrasonic_move = 0;
bool ultra_sonic_moving_left = false;
bool ultra_sonic_moving_right = false;
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

int CheckDistance() {
  digitalWrite(ULTRASONIC_PIN_TRIGGER, LOW);   
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_PIN_TRIGGER, HIGH);  
  delayMicroseconds(20);
  digitalWrite(ULTRASONIC_PIN_TRIGGER, LOW);   
  float Fdistance = pulseIn(ULTRASONIC_PIN_ECHO, HIGH);  
  Fdistance= Fdistance/58;       
  return (int)Fdistance;
}  

IRrecv ir_receiver(IR_PIN);
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

  // setup ultrasonic pins
  pinMode(ULTRASONIC_PIN_ECHO, INPUT);    
  pinMode(ULTRASONIC_PIN_TRIGGER, OUTPUT);

  ultrasonic_servo.attach(ULTRASONIC_SERVO_PIN);
  SetUltrasonicPos(90);

  // setup ir
  ir_receiver.enableIRIn();

  digitalWrite(13, LOW);
}

bool line_tracking = false;

void IrLoop() {
  if (ir_receiver.decode()) {
    unsigned long msg = ir_receiver.decodedIRData.command;
    ir_receiver.resume();

    Serial.println(msg);
    if (msg == 70) {
      if(!line_tracking) {
        FORWARD;
      }
    } else if (msg == 21) {
      if(!line_tracking) {
        BACK;
      }
    } else if (msg == 68) {
      if(!line_tracking) {
        LEFT;
      }
    } else if (msg == 67) {
      if(!line_tracking) {
        RIGHT;
      }
    } else if (msg == 64) {
      if(!line_tracking) {
        STOP;
      }
    } else if (msg == 66) {
      ULTRASONIC_LEFT;
    } else if (msg == 74) {
      ULTRASONIC_RIGHT;
    } else if (msg == 82) {
      ULTRASONIC_STOP;
    } else if (msg == 22) {
      ULTRASONIC_RESET;
    } else if (msg == 25) {
      STOP;
      line_tracking = true;
    } else if (msg == 13) {
      line_tracking = false;
      STOP;
    }
  }
}

void MsgLoop() {
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
        if(!line_tracking) {
          FORWARD;
        }
      } else if(msg == "back") {
        if(!line_tracking) {
          BACK;
        }
      } else if(msg == "left") {
        if(!line_tracking) {
          LEFT;
        }
      } else if(msg == "right") {
        if(!line_tracking) {
          RIGHT;
        }
      } else if(msg == "stop") {
        if(!line_tracking) {
          STOP;
        }
      } else if(msg == "ultra-left") {
        ULTRASONIC_LEFT;
      } else if(msg == "ultra-right") {
        ULTRASONIC_RIGHT;
      } else if(msg == "ultra-stop") {
        ULTRASONIC_STOP;
      } else if(msg == "ultra-reset") {
        ULTRASONIC_RESET;
      } else if(msg == "line-track-start") {
        STOP;
        line_tracking = true;
      } else if(msg == "line-track-stop") {
        line_tracking = false;
        STOP;
      } else {
        StaticJsonDocument<80> error;
        error["error"] = "Unknown event type";
        serializeJson(error, Serial);
      }
    }
    digitalWrite(13, LOW);
  }
}

unsigned long time_since_update = 0;
void loop() {
  if (millis() - time_since_ultrasonic_move >= 20) {
    if(ultra_sonic_moving_left) {
      IncreaseUltrasonicPos(1);
    } else if(ultra_sonic_moving_right) {
      DecreaseUltrasonicPos(1);
    }
    time_since_ultrasonic_move = millis();
  }
  if (line_tracking) {
    if(LINE_TRACKING_2) {
      FORWARD;
    } else if(LINE_TRACKING_1) { 
      LEFT;
      while(LINE_TRACKING_1);
      FORWARD;                          
    }   
    else if(LINE_TRACKING_3) {
      RIGHT;
      while(LINE_TRACKING_3); 
      FORWARD;
    }
  }

  IrLoop();
  MsgLoop();

  if (millis() - time_since_update >= 500) {
    {
      StaticJsonDocument<128> json;
      json["type"] = "status";
      json["ultrasonic"]["pos"] = GetUltrasonicPos();
      json["ultrasonic"]["distance"] = CheckDistance();
      json["linetracking"][0] = LINE_TRACKING_1 ? true : false;
      json["linetracking"][1] = LINE_TRACKING_2 ? true : false;
      json["linetracking"][2] = LINE_TRACKING_3 ? true : false;
      serializeJson(json, Serial);
    }
    time_since_update = millis();
  }
}