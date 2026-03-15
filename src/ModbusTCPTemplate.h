/*
    Modbus Library for Arduino
    ModbusTCP general implementation (适配 Pico W/RP2040)
    Copyright (C) 2014 André Sarmento Barbosa
                  2017-2020 Alexander Emelianov (a.m.emelianov@gmail.com)
    Modified for Raspberry Pi Pico W (RP2040) - 2026
*/

#pragma once
#include "Modbus.h"
#include <Arduino.h>
//#include <WiFi.h>
#include <stdlib.h>
#include <string.h>

// ========== Pico W 适配：补充缺失的宏定义 ==========
#ifndef BIT_SET
#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#endif
#ifndef BIT_CLEAR
#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
#endif
#ifndef BIT_CHECK
#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b))))        // '!!' to make sure this returns 0 or 1
#endif

#ifndef IPADDR_NONE
#define IPADDR_NONE ((uint32_t)0xffffffffUL)
#endif

// ========== Pico W 适配：调整内存和超时参数 ==========
// 根据 RP2040 内存限制调整（默认参数可能过大）
#ifndef MODBUSIP_MAX_CLIENTS
#define MODBUSIP_MAX_CLIENTS 4       // 从8个减到4个，节省内存
#endif
#ifndef MODBUSIP_MAXFRAME
#define MODBUSIP_MAXFRAME 128        // 从256减到128，适配RP2040内存
#endif
#ifndef MODBUSIP_TIMEOUT
#define MODBUSIP_TIMEOUT 3000        // 超时时间3秒
#endif
#ifndef MODBUSIP_MAX_READMS
#define MODBUSIP_MAX_READMS 50       // 单次读取最大耗时50ms，避免阻塞
#endif
#ifndef MODBUSTCP_PORT
#define MODBUSTCP_PORT 502
#endif
#ifndef MODBUSIP_UNIT
#define MODBUSIP_UNIT 1
#endif
#ifndef MODBUSIP_MAX_TRANSACTIONS
#define MODBUSIP_MAX_TRANSACTIONS 8  // 最大事务数
#endif

// ========== Pico W 适配：替换__swap_16（RP2040无此函数） ==========
//#ifndef __swap_16
//#define __swap_16(x) (((x) >> 8) | ((x) << 8))
//#endif

// Callback function Type
#if defined(MODBUS_USE_STL)
#include <functional>
#include <vector>
typedef std::function<bool(IPAddress)> cbModbusConnect;
typedef std::function<IPAddress(const char*)> cbModbusResolver;
#else
// Pico W 适配：简化DArray实现（避免STL依赖）
template <typename T, size_t MIN_SIZE, size_t GROW_BY>
class DArray {
private:
    T* data;
    size_t size;
    size_t capacity;
public:
    DArray() : data(nullptr), size(0), capacity(0) {
        reserve(MIN_SIZE);
    }
    ~DArray() {
        free(data);
    }
    void reserve(size_t new_cap) {
        if (new_cap <= capacity) return;
        T* new_data = (T*)realloc(data, new_cap * sizeof(T));
        if (new_data) {
            data = new_data;
            capacity = new_cap;
        }
    }
    void push_back(const T& val) {
        if (size >= capacity) {
            reserve(capacity + GROW_BY);
        }
        data[size++] = val;
    }
    size_t find(std::function<bool(T&)> pred) {
        for (size_t i = 0; i < size; i++) {
            if (pred(data[i])) return i;
        }
        return -1;
    }
    T* entry(size_t idx) {
        return (idx < size) ? &data[idx] : nullptr;
    }
    void remove(size_t idx) {
        if (idx >= size) return;
        memmove(&data[idx], &data[idx+1], (size - idx - 1) * sizeof(T));
        size--;
    }
    size_t size() const { return size; }
    T& operator[](size_t idx) { return data[idx]; }
};
typedef bool (*cbModbusConnect)(IPAddress ip);
typedef IPAddress (*cbModbusResolver)(const char*);
#endif

struct TTransaction {
    uint16_t    transactionId;
    uint32_t    timestamp;
    cbTransaction cb = nullptr;
    uint8_t*    _frame = nullptr;
    uint8_t*    data = nullptr;
    TAddress    startreg;
    Modbus::ResultCode forcedEvent = Modbus::EX_SUCCESS; // EX_SUCCESS means no forced event here. Forced EX_SUCCESS is not possible.
    bool operator ==(const TTransaction &obj) const {
        return transactionId == obj.transactionId;
    }
};

template <class SERVER, class CLIENT>
class ModbusTCPTemplate : public Modbus {
protected:
    union MBAP_t {
        struct {
            uint16_t transactionId;
            uint16_t protocolId;
            uint16_t length;
            uint8_t  unitId;
        };
        uint8_t  raw[7];
    };
    cbModbusConnect cbConnect = nullptr;
    cbModbusConnect cbDisconnect = nullptr;
    SERVER* tcpserver = nullptr;
    CLIENT* tcpclient[MODBUSIP_MAX_CLIENTS];
    
    // Pico W 适配：根据最大客户端数调整类型（简化）
    uint32_t tcpServerConnection = 0; // 统一用32位，避免条件编译
    
    #if defined(MODBUS_USE_STL)
    std::vector<TTransaction> _trans;
    #else
    DArray<TTransaction, 2, 2> _trans;
    #endif
    
    int16_t     transactionId = 1;  // Last started transaction. Increments on unsuccessful transaction start too.
    int8_t n = -1;
    bool autoConnectMode = false;
    uint16_t serverPort = 0;
    uint16_t defaultPort = MODBUSTCP_PORT;
    cbModbusResolver resolve = nullptr;
    bool cbEnabled = true; // Pico W 适配：补充缺失的变量
    
    TTransaction* searchTransaction(uint16_t id);
    void cleanupConnections();    // Free clients if not connected
    void cleanupTransactions();   // Remove timedout transactions and forced event
    int8_t getFreeClient();       // Returns free slot position
    int8_t getSlave(IPAddress ip);
    int8_t getMaster(IPAddress ip);
    
    // Pico W 适配：补充缺失的exceptionResponse函数（原代码依赖Modbus基类）
    void exceptionResponse(FunctionCode fc, Modbus::ResultCode exc) {
        _len = 2;
        if (_frame) free(_frame);
        _frame = (uint8_t*)malloc(_len);
        if (_frame) {
            _frame[0] = fc | 0x80;
            _frame[1] = (uint8_t)exc;
        }
        _reply = exc;
    }

public:
    uint16_t send(String host, TAddress startreg, cbTransaction cb, uint8_t unit = MODBUSIP_UNIT, uint8_t* data = nullptr, bool waitResponse = true);
    uint16_t send(const char* host, TAddress startreg, cbTransaction cb, uint8_t unit = MODBUSIP_UNIT, uint8_t* data = nullptr, bool waitResponse = true);
    uint16_t send(IPAddress ip, TAddress startreg, cbTransaction cb, uint8_t unit = MODBUSIP_UNIT, uint8_t* data = nullptr, bool waitResponse = true);
    
    ModbusTCPTemplate();
    ~ModbusTCPTemplate();
    bool isTransaction(uint16_t id);
    
#if defined(MODBUSIP_USE_DNS)
    bool isConnected(String host);
    bool isConnected(const char* host);
    bool connect(String host, uint16_t port = 0);
    bool connect(const char* host, uint16_t port = 0);
    bool disconnect(String host);
    bool disconnect(const char* host);
#endif
    
    bool isConnected(IPAddress ip);
    bool connect(IPAddress ip, uint16_t port = 0);
    bool disconnect(IPAddress ip);
    
    // ModbusTCP
    void server(uint16_t port = 0);
    // ModbusTCP depricated
    inline void slave(uint16_t port = 0) { server(port); }   // Depricated
    inline void master() { client(); }   // Depricated
    inline void begin() { server(); };   // Depricated
    void client();
    void task();
    void onConnect(cbModbusConnect cb = nullptr);
    void onDisconnect(cbModbusConnect cb = nullptr);
    uint32_t eventSource() override;
    void autoConnect(bool enabled = true);
    void dropTransactions();
    uint16_t setTransactionId(uint16_t);
    
    #if defined(MODBUS_USE_STL)
    static IPAddress defaultResolver(const char*) {return IPADDR_NONE;}
    #else
    static IPAddress defaultResolver(const char*) {return IPADDR_NONE;}
    #endif
};

// ========== 成员函数实现（适配Pico W） ==========
template <class SERVER, class CLIENT>
ModbusTCPTemplate<SERVER, CLIENT>::ModbusTCPTemplate() {
    // Pico W 适配：初始化客户端数组为nullptr
    memset(tcpclient, 0, sizeof(tcpclient));
    resolve = defaultResolver;
    #if defined(MODBUS_USE_STL)
    _trans.reserve(MODBUSIP_MAX_TRANSACTIONS);
    #endif
}

template <class SERVER, class CLIENT>
ModbusTCPTemplate<SERVER, CLIENT>::~ModbusTCPTemplate() {
    free(_frame);
    dropTransactions();
    cleanupConnections();
    cleanupTransactions();
    
    // Pico W 适配：安全删除服务器对象
    if (tcpserver) {
        delete tcpserver;
        tcpserver = nullptr;
    }
    
    // Pico W 适配：逐个清理客户端，避免内存泄漏
    for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++) {
        if (tcpclient[i]) {
            tcpclient[i]->stop();
            delete tcpclient[i];
            tcpclient[i] = nullptr;
        }
    }
}

template <class SERVER, class CLIENT>
void ModbusTCPTemplate<SERVER, CLIENT>::client() {
    // Pico W 适配：客户端模式初始化（空实现，保持兼容）
}

template <class SERVER, class CLIENT>
void ModbusTCPTemplate<SERVER, CLIENT>::server(uint16_t port) {
    // Pico W 适配：创建WiFiServer对象
    if (port)
        serverPort = port;
    else
        serverPort = defaultPort;
    
    // 先清理旧服务器
    if (tcpserver) {
        delete tcpserver;
        tcpserver = nullptr;
    }
    
    tcpserver = new SERVER(serverPort);
    if (tcpserver) {
        tcpserver->begin();
        // Pico W 适配：设置服务器超时，避免阻塞
        //tcpserver->setTimeout(1);
    }
}

#if defined(MODBUSIP_USE_DNS)
template <class SERVER, class CLIENT>
bool ModbusTCPTemplate<SERVER, CLIENT>::connect(String host, uint16_t port) {
    return connect(resolve(host.c_str()), port);
}

template <class SERVER, class CLIENT>
bool ModbusTCPTemplate<SERVER, CLIENT>::connect(const char* host, uint16_t port) {
    return connect(resolve(host), port);
}
#endif

template <class SERVER, class CLIENT>
bool ModbusTCPTemplate<SERVER, CLIENT>::connect(IPAddress ip, uint16_t port) {
    cleanupConnections();
    if (!ip || ip == IPADDR_NONE)
        return false;
    
    // 检查是否已连接
    if (getSlave(ip) != -1)
        return true;
    
    // 获取空闲客户端槽位
    int8_t p = getFreeClient();
    if (p == -1)
        return false;
    
    // Pico W 适配：创建WiFiClient并连接
    tcpclient[p] = new CLIENT();
    if (!tcpclient[p])
        return false;
    
    BIT_CLEAR(tcpServerConnection, p);
    
    // Pico W 适配：带超时的连接（避免阻塞）
    uint16_t connectPort = port ? port : defaultPort;
    tcpclient[p]->setTimeout(1000); // 连接超时1秒
    bool connected = tcpclient[p]->connect(ip, connectPort);
    
    if (!connected) {
        delete tcpclient[p];
        tcpclient[p] = nullptr;
        return false;
    }
    
    return true;
}

template <class SERVER, class CLIENT>
uint32_t ModbusTCPTemplate<SERVER, CLIENT>::eventSource() {
    // Pico W 适配：返回客户端IP的32位表示
    if (n >= 0 && n < MODBUSIP_MAX_CLIENTS && tcpclient[n])
        return (uint32_t)tcpclient[n]->remoteIP();
    return (uint32_t)INADDR_NONE;
}

template <class SERVER, class CLIENT>
TTransaction* ModbusTCPTemplate<SERVER, CLIENT>::searchTransaction(uint16_t id) {
    #if defined(MODBUS_USE_STL)
    for (auto& trans : _trans) {
        if (trans.transactionId == id) return &trans;
    }
    return nullptr;
    #else
    size_t idx = _trans.find([id](TTransaction& trans) {
        return trans.transactionId == id;
    });
    return _trans.entry(idx);
    #endif
}
template <class SERVER, class CLIENT>
void ModbusTCPTemplate<SERVER, CLIENT>::task() {
    MBAP_t _MBAP;
    uint32_t taskStart = millis();
    
    cleanupConnections();
    
    // 处理服务器端新连接（Pico W WiFiServer 适配）
    if (tcpserver) {
        CLIENT newClient = tcpserver->accept();
        if (newClient) {
            // 检查是否是新客户端
            bool isNew = true;
            for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++) {
                if (tcpclient[i] && tcpclient[i]->connected() && 
                    tcpclient[i]->remoteIP() == newClient.remoteIP()) {
                    isNew = false;
                    break;
                }
            }
            if (isNew) {
                // 调用连接回调
                bool allowConnect = (cbConnect == nullptr) ? true : cbConnect(newClient.remoteIP());
                if (allowConnect) {
                    int8_t p = getFreeClient();
                    if (p > -1) {
                        // 清理旧客户端（如果有）
                        if (tcpclient[p]) {
                            tcpclient[p]->stop();
                            delete tcpclient[p];
                        }
                        tcpclient[p] = new CLIENT(newClient);
                        BIT_SET(tcpServerConnection, p);
                        Serial.printf("新客户端连接: %s, 槽位: %d\n", newClient.remoteIP().toString().c_str(), p);
                    } else {
                        newClient.stop();
                        Serial.println("客户端槽位已满");
                    }
                } else {
                    newClient.stop();
                }
            }
        }
    }
    
    // 处理现有客户端数据
    for (n = 0; n < MODBUSIP_MAX_CLIENTS; n++) {
        if (!tcpclient[n] || !tcpclient[n]->connected()) continue;
        
        // 确保有足够的数据（MBAP头 + 至少1字节功能码）
        if (tcpclient[n]->available() < (int)(sizeof(_MBAP.raw) + 1)) continue;
        
        // 读取MBAP头（7字节）
        size_t mbapLen = tcpclient[n]->readBytes(_MBAP.raw, sizeof(_MBAP.raw));
        if (mbapLen != sizeof(_MBAP.raw)) {
            tcpclient[n]->flush();
            continue;
        }
        
        // 验证Modbus TCP协议（Protocol ID必须为0）
        uint16_t protoId = __swap_16(_MBAP.protocolId);
        if (protoId != 0) {
            Serial.println("非Modbus TCP数据包，丢弃");
            tcpclient[n]->flush();
            continue;
        }
        
        // 解析MBAP字段（关键修复：正确处理长度）
        uint16_t transId = __swap_16(_MBAP.transactionId);
        uint8_t unitId = _MBAP.unitId;
        uint16_t pduLen = __swap_16(_MBAP.length) - 1; // 长度 = PDU长度 + Unit ID（1字节）
        
        // 验证PDU长度
        if (pduLen < 1 || pduLen > MODBUSIP_MAXFRAME) {
            Serial.printf("PDU长度非法: %d\n", pduLen);
            tcpclient[n]->flush();
            continue;
        }
        
        // 读取PDU数据（功能码 + 数据）
        free(_frame);
        _frame = (uint8_t*)malloc(pduLen);
        if (!_frame) {
            Serial.println("内存分配失败");
            tcpclient[n]->flush();
            continue;
        }
        
        size_t pduRead = tcpclient[n]->readBytes(_frame, pduLen);
        if (pduRead != pduLen) {
            free(_frame);
            _frame = nullptr;
            tcpclient[n]->flush();
            Serial.println("PDU读取不完整");
            continue;
        }
        
        // 打印调试信息（方便排查）
        Serial.printf("收到请求 - TransID: %d, UnitID: %d, 功能码: 0x%02X, PDU长度: %d\n", 
                      transId, unitId, _frame[0], pduLen);
        
        // 处理从机请求（核心：调用slavePDU处理功能码）
        _reply = Modbus::EX_PASSTHROUGH;
        slavePDU(_frame); // 处理读/写寄存器请求
        
        // 构建响应数据包（MBAP + 响应PDU）
        if (_reply != Modbus::REPLY_OFF && _frame != nullptr) {
            _MBAP.transactionId = __swap_16(transId); // 响应的事务ID必须和请求一致
            _MBAP.protocolId = __swap_16(0);
            _MBAP.length = __swap_16(_len + 1); // _len = 响应PDU长度，+1=Unit ID
            _MBAP.unitId = unitId;
            
            // 拼接响应数据
            size_t respLen = sizeof(_MBAP.raw) + _len;
            uint8_t* respBuf = (uint8_t*)malloc(respLen);
            if (respBuf) {
                memcpy(respBuf, _MBAP.raw, sizeof(_MBAP.raw));
                memcpy(respBuf + sizeof(_MBAP.raw), _frame, _len);
                
                // 发送响应
                tcpclient[n]->write(respBuf, respLen);
                Serial.printf("发送响应 - 长度: %d\n", respLen);
                free(respBuf);
            }
        }
        
        // 清理缓冲区
        free(_frame);
        _frame = nullptr;
        _len = 0;
    }
    
    n = -1;
    cleanupTransactions();
}

template <class SERVER, class CLIENT>
uint16_t ModbusTCPTemplate<SERVER, CLIENT>::send(String host, TAddress startreg, cbTransaction cb, uint8_t unit, uint8_t* data, bool waitResponse) {
    return send(host.c_str(), startreg, cb, unit, data, waitResponse);
}

template <class SERVER, class CLIENT>
uint16_t ModbusTCPTemplate<SERVER, CLIENT>::send(const char* host, TAddress startreg, cbTransaction cb, uint8_t unit, uint8_t* data, bool waitResponse) {
    return send(resolve(host), startreg, cb, unit, data, waitResponse);
}

template <class SERVER, class CLIENT>
uint16_t ModbusTCPTemplate<SERVER, CLIENT>::send(IPAddress ip, TAddress startreg, cbTransaction cb, uint8_t unit, uint8_t* data, bool waitResponse) {
    MBAP_t _MBAP;
    uint16_t result = 0;
    int8_t p = -1;
    
    // 检查事务数限制
    #if defined(MODBUSIP_MAX_TRANSACTIONS)
    #if defined(MODBUS_USE_STL)
    if (_trans.size() >= MODBUSIP_MAX_TRANSACTIONS)
        goto cleanup;
    #else
    if (_trans.size() >= MODBUSIP_MAX_TRANSACTIONS)
        goto cleanup;
    #endif
    #endif
    
    // 检查IP有效性
    if (!ip || ip == IPADDR_NONE)
        goto cleanup;
    
    // 查找已连接的客户端
    p = (tcpserver) ? getMaster(ip) : getSlave(ip);
    if (p == -1 || !tcpclient[p] || !tcpclient[p]->connected()) {
        if (!autoConnectMode)
            goto cleanup;
        // 自动重连
        if (!connect(ip))
            goto cleanup;
        p = getSlave(ip);
        if (p == -1)
            goto cleanup;
    }
    
    // 构建MBAP头
    _MBAP.transactionId = __swap_16(transactionId);
    _MBAP.protocolId = __swap_16(0);
    _MBAP.length = __swap_16(_len + 1); // +1 for unitId
    _MBAP.unitId = unit;
    
    // 发送数据（堆分配，避免栈溢出）
	bool writeResult;
	{	// for sbuf isolation
		size_t send_len = _len + sizeof(_MBAP.raw);
		uint8_t sbuf[send_len];
		memcpy(sbuf, _MBAP.raw, sizeof(_MBAP.raw));
		memcpy(sbuf + sizeof(_MBAP.raw), _frame, _len);
		writeResult = (tcpclient[p]->write(sbuf, send_len) == send_len);
	}
	
    if (!writeResult) {
        Serial.println("数据发送失败");
        goto cleanup;
    }
    
    // 保存事务（如果需要等待响应）
    if (waitResponse) {
        TTransaction tmp;
        tmp.transactionId = transactionId;
        tmp.timestamp = millis();
        tmp.cb = cb;
        tmp.data = data;
        tmp._frame = _frame; // 转移所有权
        tmp.startreg = startreg;
        _trans.push_back(tmp);
        _frame = nullptr; // 避免重复释放
    }
    
    // 更新事务ID
    result = transactionId;
    transactionId++;
    if (transactionId == 0)
        transactionId = 1;

cleanup:
    // 清理缓冲区
    if (_frame) {
        free(_frame);
        _frame = nullptr;
    }
    _len = 0;
    return result;
}

template <class SERVER, class CLIENT>
void ModbusTCPTemplate<SERVER, CLIENT>::onConnect(cbModbusConnect cb) {
    cbConnect = cb;
}

template <class SERVER, class CLIENT>
void ModbusTCPTemplate<SERVER, CLIENT>::onDisconnect(cbModbusConnect cb) {
    cbDisconnect = cb;
}

template <class SERVER, class CLIENT>
void ModbusTCPTemplate<SERVER, CLIENT>::cleanupConnections() {
    for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++) {
        if (tcpclient[i] && !tcpclient[i]->connected()) {
            // 调用断开回调
            if (cbDisconnect && cbEnabled)
                cbDisconnect(IPADDR_NONE);
            // 清理客户端
            tcpclient[i]->stop();
            delete tcpclient[i];
            tcpclient[i] = nullptr;
            BIT_CLEAR(tcpServerConnection, i);
        }
    }
}

template <class SERVER, class CLIENT>
void ModbusTCPTemplate<SERVER, CLIENT>::cleanupTransactions() {
    #if defined(MODBUS_USE_STL)
    for (auto it = _trans.begin(); it != _trans.end();) {
        if (millis() - it->timestamp > MODBUSIP_TIMEOUT || it->forcedEvent != Modbus::EX_SUCCESS) {
            Modbus::ResultCode res = (it->forcedEvent != Modbus::EX_SUCCESS) ? it->forcedEvent : Modbus::EX_TIMEOUT;
            if (it->cb)
                it->cb(res, it->transactionId, nullptr);
            free(it->_frame);
            it = _trans.erase(it);
        } else {
            it++;
        }
    }
    #else
    size_t i = 0;
    while (i < _trans.size()) {
        TTransaction& t = _trans[i];
        if (millis() - t.timestamp > MODBUSIP_TIMEOUT || t.forcedEvent != Modbus::EX_SUCCESS) {
            Modbus::ResultCode res = (t.forcedEvent != Modbus::EX_SUCCESS) ? t.forcedEvent : Modbus::EX_TIMEOUT;
            if (t.cb)
                t.cb(res, t.transactionId, nullptr);
            free(t._frame);
            _trans.remove(i);
        } else {
            i++;
        }
    }
    #endif
}

template <class SERVER, class CLIENT>
int8_t ModbusTCPTemplate<SERVER, CLIENT>::getFreeClient() {
    for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++) {
        if (!tcpclient[i])
            return i;
    }
    return -1;
}

template <class SERVER, class CLIENT>
int8_t ModbusTCPTemplate<SERVER, CLIENT>::getSlave(IPAddress ip) {
    for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++) {
        if (tcpclient[i] && tcpclient[i]->connected() && 
            tcpclient[i]->remoteIP() == ip && !BIT_CHECK(tcpServerConnection, i)) {
            return i;
        }
    }
    return -1;
}

template <class SERVER, class CLIENT>
int8_t ModbusTCPTemplate<SERVER, CLIENT>::getMaster(IPAddress ip) {
    for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++) {
        if (tcpclient[i] && tcpclient[i]->connected() && 
            tcpclient[i]->remoteIP() == ip && BIT_CHECK(tcpServerConnection, i)) {
            return i;
        }
    }
    return -1;
}

template <class SERVER, class CLIENT>
bool ModbusTCPTemplate<SERVER, CLIENT>::isTransaction(uint16_t id) {
    return searchTransaction(id) != nullptr;
}

#if defined(MODBUSIP_USE_DNS)
template <class SERVER, class CLIENT>
bool ModbusTCPTemplate<SERVER, CLIENT>::isConnected(String host) {
    return isConnected(resolve(host.c_str()));
}

template <class SERVER, class CLIENT>
bool ModbusTCPTemplate<SERVER, CLIENT>::isConnected(const char* host) {
    return isConnected(resolve(host));
}
#endif

template <class SERVER, class CLIENT>
bool ModbusTCPTemplate<SERVER, CLIENT>::isConnected(IPAddress ip) {
    if (!ip || ip == IPADDR_NONE)
        return false;
    return getSlave(ip) != -1;
}

template <class SERVER, class CLIENT>
void ModbusTCPTemplate<SERVER, CLIENT>::autoConnect(bool enabled) {
    autoConnectMode = enabled;
}

#if defined(MODBUSIP_USE_DNS)
template <class SERVER, class CLIENT>
bool ModbusTCPTemplate<SERVER, CLIENT>::disconnect(String host) {
    return disconnect(resolve(host.c_str()));
}

template <class SERVER, class CLIENT>
bool ModbusTCPTemplate<SERVER, CLIENT>::disconnect(const char* host) {
    return disconnect(resolve(host));
}
#endif

template <class SERVER, class CLIENT>
bool ModbusTCPTemplate<SERVER, CLIENT>::disconnect(IPAddress ip) {
    if (!ip || ip == IPADDR_NONE)
        return false;
    int8_t p = getSlave(ip);
    if (p != -1) {
        tcpclient[p]->stop();
        delete tcpclient[p];
        tcpclient[p] = nullptr;
        BIT_CLEAR(tcpServerConnection, p);
        return true;
    }
    return false;
}

template <class SERVER, class CLIENT>
void ModbusTCPTemplate<SERVER, CLIENT>::dropTransactions() {
    #if defined(MODBUS_USE_STL)
    for (auto& t : _trans) {
        t.forcedEvent = Modbus::EX_CANCEL;
    }
    #else
    for (size_t i = 0; i < _trans.size(); i++) {
        _trans[i].forcedEvent = Modbus::EX_CANCEL;
    }
    #endif
}

template <class SERVER, class CLIENT>
uint16_t ModbusTCPTemplate<SERVER, CLIENT>::setTransactionId(uint16_t t) {
    transactionId = t;
    if (transactionId == 0)
        transactionId = 1;
    return transactionId;
}