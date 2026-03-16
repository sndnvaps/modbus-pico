# Modbus Library for Arduino
### ModbusRTU

For detailes on the library usage visit [documentation](documentation) section.

## Features

## This project now port to support Raspberry PICO/PICOW/PICO2W...(rp2040/rp2350)

## for PICO,  port ModbusRTU ,ModbusTCP, ModbusEthernet


 - ModbusRTU_PICO.h for usb-serial or serial-ttl connect
 - ModbusIP_PICO.h for WiFi connect, use WiFi.h on Arduino-pico(for Picow & pico2w)
 - ModbusEthernet.h for Ethernet v2 support (not work for lwip-ethernet by my test)
 - ModbusTLS.h for WiFi tls connect, use WiFi.h on Arduino-pico(for Picow & pico2w)


## Example

```
//modbusRTU_pico_example.ino
#include "ModbusRTU_PICO.h"

#define MODBUS_SLAVE_ID 1
#define REG_TEMPERATURE 0  // 温度×10 寄存器地址
#define REG_HUMIDITY 1     // 湿度×10 寄存器地址

ModbusRTU mb;

// 模拟温湿度数据（存储在寄存器中）
uint16_t temperature = 256;  // 25.6℃（放大10倍存储）
uint16_t humidity = 605;     // 60.5%（放大10倍存储）

void setup() {
  // 初始化 Modbus 从站
  Serial.begin(9600, SERIAL_8N1);
  mb.begin(&Serial);
  //mb.setBaudrate(9600);
  mb.slave(MODBUS_SLAVE_ID);
  mb.addHreg(REG_TEMPERATURE);
  mb.addHreg(REG_HUMIDITY);
}

void loop() {

  // 模拟数据更新（每1秒更新一次温湿度）
  static uint32_t lastUpdate = 0;
  if (millis() - lastUpdate >= 2000) {
    lastUpdate = millis();
    // 温度随机波动±0.5℃
    temperature = constrain(temperature + (random(-5, 6)), 200, 300);
    // 湿度随机波动±0.5%
    humidity = constrain(humidity + (random(-5, 6)), 500, 700);
    // 更新寄存器
  
    mb.Hreg(REG_TEMPERATURE, temperature);
    mb.Hreg(REG_HUMIDITY, humidity);

    // 调试输出
    //Serial1.printf("Temperature: %.1f℃, Humidity: %.1f%%\n",
    //               temperature/10.0, humidity/10.0);
  }
  //delay(1);
  mb.task();
}
```

* Supports all Arduino platforms
* Operates in any combination of multiple instances of
  * [Modbus RTU server](examples/RTU)
  * [Modbus RTU client](examples/RTU)
  * Modbus TCP server for [PICO](examples/TCP) and [Ethernet library](examples/TCP-Ethernet)
  * Modbus TCP client for [PICOW/PICO2W](examples/TCP-PICO) and [Ethernet library](examples/TCP-Ethernet)
  * [MODBUS/TCP Security server (PICO)](examples/TLS)
  * [MODBUS/TCP Security client (PICOW/PICO2W)](examples/TLS)
* Modbus functions supported:
  * 0x01 - Read Coils
  * 0x02 - Read Input Status (Read Discrete Inputs)
  * 0x03 - Read Holding Registers
  * 0x04 - Read Input Registers
  * 0x05 - Write Single Coil
  * 0x06 - Write Single Register
  * 0x0F - Write Multiple Coils
  * 0x10 - Write Multiple Registers
  * 0x14 - Read File Record
  * 0x15 - Write File Record
  * 0x16 - Mask Write Register
  * 0x17 - Read/Write multiple registers
* [Callbacks](examples/Callback) driven design
* Real life complex examples:
  * [ESP8266/ESP32 firmware update over Modbus](examples/Files)
  * [ModbusRTU to ModbusTCP bridge](examples/bridge)

## Notes

1. The offsets for registers are 0-based. So be careful when setting your supervisory system or your testing software. For example, in [ScadaBR](http://www.scadabr.com.br) offsets are 0-based, then, a register configured as 100 in the library is set to 100 in ScadaBR. On the other hand, in the [CAS Modbus Scanner](http://www.chipkin.com/products/software/modbus-software/cas-modbus-scanner/) offsets are 1-based, so a register configured as 100 in library should be 101 in this software.
2. RS-485 transivers based on MAX-485 is working on at least up to 115200. XY-017/XY-485 working only up to 9600 for some reason.

For more information about Modbus see:

* [Modbus (From Wikipedia, the free encyclopedia)](http://pt.wikipedia.org/wiki/Modbus)
* [MODBUS APPLICATION PROTOCOL SPECIFICATION V1.1b3](https://modbus.org/docs/Modbus_Application_Protocol_V1_1b3.pdf)
* [MODBUS MESSAGING ON TCP/IP IMPLEMENTATION GUIDE V1.0b](http://www.modbus.org/docs/Modbus_Messaging_Implementation_Guide_V1_0b.pdf)
* [MODBUS over Serial Line Specification and Implementation Guide V1.02](http://www.modbus.org/docs/Modbus_over_serial_line_V1_02.pdf)
* [MODBUS/TCP Security Protocol Specification](https://modbus.org/docs/MB-TCP-Security-v21_2018-07-24.pdf)

## Last Changes

```diff
// 4.1.0
+ API: Raw Modbus frame processing functionality
+ ModbusRTU: Precise inter-frame interval control
+ Examples: True ModbusRTU to ModbusTCP Server bridge
+ Examples: ModbusRTU respond to multiple ID from single device
+ ModbusRTU: Add direction control pin for Stream
+ STL: Add Reg count limitation to vector limit of 4000 (for ESP8266 and ESP32)
+ Settings: Added MODBUSIP_CONNECTION_TIMEOUT (ESP32 only)
+ Settings: Set MODBUSIP_MAX_CLIENTS = 8 for ESP32
+ ModbusTCP: Make using DNS names optional feature
+ ModbusRTU: Add separate RE/DE pins control optional feature
+ API: Drop support of Ethernet library v1
+ Examples: Teknic ClearCore ArduinoWrapper examples added
+ Examples: ModbusTCP to ModbusRTU example added
+ ModbusRTU: Flush extra delay optional feature
// 4.0.0
+ Support of all Arduino boards
+ ModbusTLS: ESP8266 Client/Server and ESP32 Client
+ ModbusTCP: ModbusEthernet - WizNet W5x00, ENC28J60 Ethernet library support
+ 0x14 - Read File Records function
+ 0x15 - Write File Records function
+ Examples: FW update over Modbus fullfunctional example
+ 0x16 - Write Mask Register function+ Test: 0x16
+ 0x17 - Read/Write Registers function
+ ModbusRTU: ESP32 SoftwareSerial support
+ Build with no STL dependency (switchable)
+ API: ModbusIP => ModbusTCP
+ API: Access control callback for individual Modbus function
+ API: Master/Slave => Client/Server according to [PRESS RELEASE](https://modbus.org/docs/Client-ServerPR-07-2020-final.docx.pdf)
+ Lot of code refacting and small fixes
```

## Roadmap

```diff
// 4.2.0-DEV
- API: Alternative CRC calulation (reduced memory footprint)
- ModbusRTU: Static buffer allocation
- Test: Frame accuracy to specefication
- Buffer/packet size limitation support
- Slave/Server: slavePDU use early exit by return where possible
- Master/Client: Check frame size against header data where possible
- Master/Client: Additional responce data validation
- Free global registers and callbacks on remove last Modbus instance
- Test: push/pull functions
- ModbusTCP: Refactor connect by dns name (using native implementation for ESP32 etc)
// 4.3.0-DEV
- ModbusTLS: ESP32 Server
- Test: TLS ESP32 Server
- Test: TLS ESP32 Client
- Examples: TLS Certificate test Role extension and Alt-Name
- Examples: TLS Add example explanation
- ModbusTCP: ModbusAsyncTCP
- API: Extend API to allow custom Modbus commands
- Examples: Basic file operations
- Examples: Revising
```
## Contributions

https://github.com/emelianov/modbus-esp8266

a.m.emelianov@gmail.com

Original version:

https://github.com/andresarmento/modbus-esp8266

https://github.com/andresarmento/modbus-arduino

prof (at) andresarmento (dot) com

## License

The code in this repo is licensed under the BSD New License. See LICENSE.txt for more info.
