/*
    Modbus Library for Arduino-pico rp2040/rp2350
    ModbusTCP for W5x00 Ethernet
    Copyright (C) 2022 Alexander Emelianov (a.m.emelianov@gmail.com)
    Copyright (C) 2026 Jimes Yang (admin@sndnvaps.com)
*/

#pragma once
#if defined(MODBUSIP_USE_DNS)
#include <Dns.h>
#endif

#if !defined(PICO_RP2040) && !defined(PICO_RP2350)
#error Unsupported architecture
#endif

#include <WiFiClientSecure.h>
#include <WiFiServerSecure.h>

#include "ModbusAPI.h"
#include "ModbusTCPTemplate.h"

#include <EthernetCompat.h> //for LWIP-Ethernet

using EthernetServerTLS = WiFiServerSecure;
using EthernetClientTLS = WiFiClientSecure;


class ModbusEthernetTLS : public ModbusAPI<ModbusTCPTemplate<EthernetServerTLS, EthernetClientTLS>> {
    private:
    int8_t _connect(IPAddress ip, uint16_t port, const char* client_cert = nullptr, const char* client_private_key = nullptr) {
	    int8_t p = getFreeClient();
	    if (p < 0)
		    return p;
	    tcpclient[p] = new EthernetClientTLS();
        BIT_CLEAR(tcpServerConnection, p);

        BearSSL::X509List *clientCertList = new BearSSL::X509List(client_cert);
        BearSSL::PrivateKey *clientPrivKey = new BearSSL::PrivateKey(client_private_key);
        tcpclient[p]->setClientRSACert(clientCertList, clientPrivKey);
        tcpclient[p]->setBufferSizes(512, 512);

        return p;
    }
#if defined(MODBUSIP_USE_DNS)
    static IPAddress resolver (const char* host) {
        IPAddress remote_addr;
        if (WiFi.hostByName(host, remote_addr))
                return remote_addr;
        return IPADDR_NONE;
    }
#endif
    public:
    ModbusEthernetTLS() : ModbusAPI() {
        defaultPort = MODBUSTLS_PORT;
#if defined(MODBUSIP_USE_DNS)
        resolve = resolver;
#endif
    }

	void server(uint16_t port, const char* server_cert = nullptr, const char* server_private_key = nullptr, const char* ca_cert = nullptr) {
        serverPort = port;
	    tcpserver = new EthernetServerTLS(serverPort);
        BearSSL::X509List *serverCertList = new BearSSL::X509List(server_cert);
        BearSSL::PrivateKey *serverPrivKey = new BearSSL::PrivateKey(server_private_key);
        tcpserver->setRSACert(serverCertList, serverPrivKey);
        if (ca_cert) {
            BearSSL::X509List *trustedCA = new BearSSL::X509List(ca_cert);
            tcpserver->setClientTrustAnchor(trustedCA);
        }
        //tcpserver->setBufferSizes(512, 512);
	    tcpserver->begin();
    }

    bool connectWithKnownKey(IPAddress ip, uint16_t port, const char* client_cert = nullptr, const char* client_private_key = nullptr, const char* key = nullptr) {
        if(getSlave(ip) >= 0)
		    return true;
        int8_t p = _connect(ip, port, client_cert, client_private_key);
        BearSSL::PublicKey *clientPublicKey = new BearSSL::PublicKey(key);
        tcpclient[p]->setKnownKey(clientPublicKey);
        return tcpclient[p]->connect(ip, port);
    }


#if defined(MODBUSIP_USE_DNS)
    bool connect(String host, uint16_t port, const char* client_cert = nullptr, const char* client_private_key = nullptr, const char* ca_cert = nullptr) {
        return connect(resolver(host.c_str()), port, client_cert, client_private_key, ca_cert);
    }
    bool connect(const char* host, uint16_t port, const char* client_cert = nullptr, const char* client_private_key = nullptr, const char* ca_cert = nullptr) {
        return connect(resolver(host), port, client_cert, client_private_key, ca_cert);
    }
#endif
    bool connect(IPAddress ip, uint16_t port, const char* client_cert = nullptr, const char* client_private_key = nullptr, const char* ca_cert = nullptr) {
        if (!ip)
            return false;
        if(getSlave(ip) >= 0)
		    return false;
        int8_t p = _connect(ip, port, client_cert, client_private_key);
        if (p < 0)
            return false;

        if (ca_cert) {
            BearSSL::X509List *trustedCA = new BearSSL::X509List(ca_cert);
            tcpclient[p]->setTrustAnchors(trustedCA);
        } else {
            tcpclient[p]->setInsecure();
        }

        //return tcpclient[p]->connect(ip, port);
        if (!tcpclient[p]->connect(ip, port))
            return false;
        return true;
    }
};
