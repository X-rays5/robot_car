//#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include "drive_defines.hpp"

#define BAUD_RATE 9600

/*void setup() {
  Serial.begin(BAUD_RATE);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  //BT16.begin(BAUD_RATE);
  Serial.println("Started");
}

//Repeat execution
void loop() {
  //if (BT16.available()) {
  //  Serial.write(BT16.read());
  //}
  //if (Serial.available()) {
  //  BT16.write(Serial.read());
  //}
  FORWARD;
  delay(1000);
  BACK;
  delay(1000);
  LEFT;
  delay(1000);
  RIGHT;
  delay(1000);
  STOP;
  delay(1000);
}*/

void setup() {
  Serial.begin(BAUD_RATE);         //Sets the data rate in bits per second (baud) for serial data transmission
  pinMode(13, OUTPUT);        //Sets digital pin 13 as output pin

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
      Serial.print(data);        //Print Value inside data in Serial monitor
      Serial.print("\n");        //New line 
    
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
      }
    }
  }                            
}
