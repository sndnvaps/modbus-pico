/*
  Modbus Library for Arduino Example - Modbus IP Client (ESP8266/ESP32)
  Read Holding Register from Modbus Server in blocking way

  (c)2020 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
*/


#include <WiFi.h>
#include <ModbusIP_PICO.h>

const int REG = 528;                 // Modbus Hreg Offset
IPAddress remote(192, 168, 30, 13);  // Address of Modbus Slave device

ModbusIP mb;  //ModbusIP object

void setup() {
  Serial.begin(115200);

  WiFi.begin("SSID", "PASSWORD");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  mb.client();
}

uint16_t res = 0;

void loop() {
  if (mb.isConnected(remote)) {                       // Check if connection to Modbus Slave is established
    uint16_t trans = mb.readHreg(remote, REG, &res);  // Initiate Read Hreg from Modbus Server
    while (mb.isTransaction(trans)) {                 // Check if transaction is active
      mb.task();
      delay(10);
    }
    Serial.println(res);  // At this point res is filled with responce value
  } else {
    mb.connect(remote);  // Try to connect if no connection
  }
  delay(100);
}