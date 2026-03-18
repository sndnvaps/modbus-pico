/*
  ModbusTCP for W5x00 Ethernet library
  Basic Client code example

  (c)2020 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266

  This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/

#include <SPI.h>
//#include <Ethernet.h>       // Ethernet library v2 is required

#include <EthernetCompat.h>  //LWIP-Ethernet
#include <ModbusLWIP_Ethernet.h>

//CS = 17
//Int = 21
ArduinoWiznet5500lwIP Ethernet(17, SPI, 21);
//ArduinoWiznet5100lwIP Ethernet(17, SPI, 21);
//ArduinoENC28J60lwIP Ethernet(17, SPI, 21);

const uint16_t REG = 0;               // Modbus Hreg Offset
IPAddress remote(192, 168, 13, 103);  // Address of Modbus Slave device
const int32_t showDelay = 2000;       // Show result every n'th mellisecond

// Enter a MAC address and IP address for your controller below.
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE
};
IPAddress ip(192, 168, 13, 178);   // The IP address will be dependent on your local network:
IPAddress myDns(192, 168, 13, 1);  //set the dns

ModbusEthernet mb;  // Declare ModbusTCP instance

void setup() {

  //SPI.setRX(16);
  //SPI.setCS(17);
  //SPI.setSCK(18);
  //SPI.setTX(19);
  //Intn = 21

  Serial.begin(115200);  // Open serial communications and wait for port to open

  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1);  // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to configure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }


  delay(1000);  // give the Ethernet shield a second to initialize
  mb.client();  // Act as Modbus TCP server
}

uint16_t res = 0;
uint32_t showLast = 0;

void loop() {
  if (mb.isConnected(remote)) {      // Check if connection to Modbus Slave is established
    mb.readHreg(remote, REG, &res);  // Initiate Read Hreg from Modbus Slave
    Serial.println(res);
    delay(1000);
  } else {
    mb.connect(remote);  // Try to connect if not connected
  }
  delay(100);                        // Pulling interval
  mb.task();                         // Common local Modbus task
  if (millis() - showLast > 1000) {  // Display register value every 5 seconds (with default settings)
    showLast = millis();
    Serial.println("HReg = ");
    Serial.println(res);
  }
}