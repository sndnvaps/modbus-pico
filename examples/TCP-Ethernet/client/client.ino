/*
  ModbusTCP for W5x00 Ethernet library
  Basic Client code example

  (c)2020 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266

  This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/

#include <SPI.h>
#include <Ethernet.h>       // Ethernet library v2 is required
#include <ModbusEthernet.h>

const uint16_t REG = 0;               // Modbus Hreg Offset
IPAddress remote(192, 168, 13, 103);  // Address of Modbus Slave device
const int32_t showDelay = 1000;   // Show result every n'th mellisecond

// Enter a MAC address and IP address for your controller below.
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 13, 105); // The IP address will be dependent on your local network:
ModbusEthernet mb;               // Declare ModbusTCP instance

void setup() {
  Serial.begin(115200);     // Open serial communications and wait for port to open
  #if defined(AVR_LEONARDO)
  while (!Serial) {}        // wait for serial port to connect. Needed for Leonardo only
  #endif
  Ethernet.init(17);         // SS pin
  // start the Ethernet connection
  Ethernet.begin(mac);


  delay(1000);              // give the Ethernet shield a second to initialize
  Serial.println("DHCP IP = ");
  Serial.println(Ethernet.localIP());
  mb.client();              // Act as Modbus TCP server
}

uint16_t res = 0;
uint32_t showLast = 0;

void loop() {
if (mb.isConnected(remote)) {   // Check if connection to Modbus Slave is established
    mb.readHreg(remote, REG, &res);  // Initiate Read Hreg from Modbus Slave
    //Serial.println(res);
    delay(1000);
  } else {
    mb.connect(remote);           // Try to connect if not connected
  }
  delay(100);                     // Pulling interval
  mb.task();                      // Common local Modbus task
  if (millis() - showLast > 1000) { // Display register value every 5 seconds (with default settings)
    showLast = millis();
    Serial.println("Hreg = 0, value = ");
    Serial.println(res);
  }
}