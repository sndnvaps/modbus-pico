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