/*
    Modbus Library for Arduino
    ModbusTCP for W5x00 Ethernet
    Copyright (C) 2022 Alexander Emelianov (a.m.emelianov@gmail.com)
*/

#pragma once
#if defined(MODBUSIP_USE_DNS)
#include <Dns.h>
#endif

#if !defined(PICO_RP2040) && !defined(PICO_RP2350)
#error Unsupported architecture
#endif

#include "ModbusAPI.h"

#include "ModbusTCPTemplate.h"

#include <WiFi.h>
#include <EthernetCompat.h> //for LWIP-Ethernet


class ModbusEthernet : public ModbusAPI<ModbusTCPTemplate<EthernetServer, EthernetClient>> {
#if defined(MODBUSIP_USE_DNS)
    private:
    static IPAddress resolver (const char* host) {
        DNSClient dns;
        IPAddress ip;
        
        dns.begin(Ethernet.dnsServerIP());
        if (dns.getHostByName(host, ip) == 1)
            return ip;
        else
            return IPADDR_NONE;
    }
    public:
    ModbusEthernet() : ModbusAPI() {
        resolve = resolver;
    }
#endif
};
