/*
    Modbus Library for Arduino
    ModbusTCP for ESP8266/ESP32
    Copyright (C) 2020 Alexander Emelianov (a.m.emelianov@gmail.com)
    Copyright (C) 2026 Jimes Yang (admin@sndnvaps.com)
*/

#pragma once
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32) || defined(PICO_RP2040) || defined(PICO_RP2350)
#include <WiFi.h>
#include <WiFiUdp.h>
#endif

#include "ModbusAPI.h"

using EthernetUDP = WiFiUDP;

#include "ModbusTCPTemplate.h"


class ModbusTCP : public ModbusAPI<ModbusTCPTemplate<WiFiServer, WiFiClient>> {
#if defined(MODBUSIP_USE_DNS)
  private:
    static IPAddress resolver(const char *host) {
        IPAddress remote_addr;
        if (WiFi.hostByName(host, remote_addr))
            return remote_addr;
        return IPADDR_NONE;
    }

  public:
    ModbusTCP() : ModbusAPI() {
        resolve = resolver;
    }
#endif
};


