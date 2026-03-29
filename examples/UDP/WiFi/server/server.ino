/*
  Modbus-Arduino Example - Test Holding Register (Modbus IP ESP8266)
  Configure Holding Register (offset 100) with initial value 0xABCD
  You can get or set this holding register
  Original library
  Copyright by André Sarmento Barbosa
  http://github.com/andresarmento/modbus-arduino

  Current version
  (c)2017 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266

  Current version
  (c) 2026 Jimes Yang (admin@sndnvaps.com)
  https://github.com/emelianov/modbus-pico

*/


#include <WiFi.h>

#include <ModbusIP_PICO.h>


#define WIFI_SSID "TP-LINK_71A109"
#define WIFI_PASSWORD "447826004aZ"

// Modbus Registers Offsets
//const int TEST_HREG = 100;
#define REG_TEMPERATURE 0  // 温度寄存器（放大10倍，25.5℃=255）
#define REG_HUMIDITY 1     // 湿度寄存器（放大10倍，60.2%=602）

// 模拟温湿度数据（存储在寄存器中）
uint16_t temperature = 256;  // 25.6℃（放大10倍存储）
uint16_t humidity = 605;     // 60.5%（放大10倍存储）


//ModbusIP object
ModbusIP mb;

// Modbus回调函数：处理寄存器读写
//Modbus::ResultCode modbusRawCallback(uint8_t* frame, uint8_t len, frame_arg_t* args) {
//  // 这里可以自定义处理逻辑
//  return Modbus::EX_PASSTHROUGH;
//}
void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  mb.setMode(MODE_UDP); //run at UDP mode
  mb.server(502);

  mb.addHreg(REG_TEMPERATURE);
  mb.addHreg(REG_HUMIDITY);

  Serial.println("Modbus TCP服务器已启动");
}

void loop() {
  //Call once inside loop() - all magic here
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
    Serial.printf("Temperature: %.1f℃, Humidity: %.1f%%\n",
                  temperature / 10.0, humidity / 10.0);
  }
  mb.task();

  delay(10);
}
