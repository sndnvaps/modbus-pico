/*
  Modbus-Arduino Example - Test Led (Modbus IP ESP8266)
  Control a Led on GPIO0 pin using Write Single Coil Modbus Function 
  Original library
  Copyright by André Sarmento Barbosa
  http://github.com/andresarmento/modbus-arduino

  Current version
  (c)2017 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
*/


#include <WiFi.h>
#include <ModbusIP_PICO.h>

//Modbus Registers Offsets
const int LED_COIL = 100;
//Used Pins
//Used LED_BUILTIN
//const int ledPin = 0; //GPIO0

//ModbusIP object
ModbusIP mb;
  
void setup() {
  Serial.begin(115200);
 
  WiFi.begin("your_ssid", "your_password");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  mb.server();

  pinMode(LED_BUILTIN, OUTPUT);
  mb.addCoil(LED_COIL);
}
 
void loop() {
   //Call once inside loop() - all magic here
   mb.task();

   //Attach ledPin to LED_COIL register
   digitalWrite(LED_BUILTIN, mb.Coil(LED_COIL));
   delay(10);
}