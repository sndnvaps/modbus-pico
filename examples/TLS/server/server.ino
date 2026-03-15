/*
  Modbus-Uni - Most complete Modbus Library for Arduino

  Modbus/TCP Security Server for ESP8266 Example
  
  (c)2020 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
  This code is licensed under the BSD New License. See LICENSE.txt for more info.
  (c) 2026 Jimes Yang (admin@sndnvaps.com)
  https://github.com/sndnvaps/modbus-pico
*/

#include <WiFi.h>
#include <time.h>
#include <ModbusTLS.h>

// The hardcoded certificate authority for this example.
// Don't use it on your own apps!!!!!

const char ca_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDEzCCAfugAwIBAgIUM3r6zWNMv+GhjWwmfo14KZLQaDswDQYJKoZIhvcNAQEL
BQAwGTEXMBUGA1UEAwwOMTkyLjE2OC4xMy4xMDMwHhcNMjYwMzE1MTIxMTQ2WhcN
MjcwMzE1MTIxMTQ2WjAZMRcwFQYDVQQDDA4xOTIuMTY4LjEzLjEwMzCCASIwDQYJ
KoZIhvcNAQEBBQADggEPADCCAQoCggEBANc+KZUNqWLXACwaKSulbqhhsnlUadZ9
RjcK2lCAbfVgafQT9KYqam+zk39z1MggFuhQTfGW859wyfDYqn9KurrTkxT7aFBs
jdmfdFL2dBkfNUMDV9Ks1bO/f/9pffQcYCP+B5OHNKr2f5UwQjIp8SIf4Q+UBjoo
wIkzixuiBJGgcDKwW4Gi8dPWV244TDYQWhTg+izv3bH8kVwk1EwjEWIXeq/EiPMJ
R9jbtGwZ2qH3haRt6ngeM0apVmISn+PvXGy3EblFleYoM4/mz9uGFeNMP+9Ay7bg
DksEOuIlGFumh+b/MwJJc334MXMBew+y6UmwHtIU7u0xRXluLpIbvkcCAwEAAaNT
MFEwHQYDVR0OBBYEFNZ4m6Mutbnuix6ZeZayDdqQD0YNMB8GA1UdIwQYMBaAFNZ4
m6Mutbnuix6ZeZayDdqQD0YNMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEL
BQADggEBAAGa/4WZsy5TAlbM3L46Yf0eFzKuGvcxU+AVuKMuxla1EYsTxF64Osx5
XEkY3Xqat9Qs/OvdplgJDjHREucEHlJ4VsrYrIcmvoCRrvT0o+n/Dk4YLKp3iOcZ
KdV/99UWAsOAAq23Pg7GSYHNHa5mV9OldPO3//USQTrEbVt86OwOdDkLIjd6QtjI
nnKGhAXNLQoRhVt8eL8a9/6BrFMHvdZayuZADb7mFa6WltxIfC3Hrsi82xDPGxHd
z2k7aCcrimxdmsgcVrlAlpoJpzFFxOzEzHquUkKdbqoqh+t3sAS/sMDbOREdDFRn
ajOnpSrawW6PU2WWYfJccpALjBbKMew=
-----END CERTIFICATE-----
)EOF";

// The server's private key which must be kept secret
const char server_private_key[] PROGMEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEogIBAAKCAQEAqpElFVLavA/4liyqALA3VpirBhbdl4934dwpsadrRb74lYMD
mCFornjcwoi8FBTmmwL0mtcBAwFGLBzsnSglrq7mrmAqdKLHr9fVMZJSFzPq3M+T
HwMfJD3ke4JsxLt4gbLqUAh235kz2Qc3RX9UcSCe/83bu8aE0N2qz5sIM3rl9Ctx
7KE2A4c9fTi423+ogT5R98gSGZnnLPNWuLdKj2PMgD3b4EEdDuBUZvxKUED6qcnJ
9VFpArPBtOSURM1NEf7gJuOhNTddAEymgmuh7VU4EXCrM2LukMJPrvMbqoYIzf6i
XVDXrK4U/AwErjUInzN1n63LcrjYU98qsTUvTwIDAQABAoIBAAuiNrtLmWu2Uyg7
B6VgX4Xih9e0e4y41l5rW9SoLKLnMaFQ/E/GB5PO3o/OL7XW7xunifN9sq33ykd7
+Y+Gi3tspoNCP+MQrzoJKJtm9X5rphtFdS1qUxdvuUPLU4vflYqTH3Qx/5mko2qs
GVL+Kk5FAVNFQTV/htOQKvRaCKcqIDWq/upoUC2BligmzbFhYH9sy6xe6KKXh7am
38kK7mf/7Kjx9nwaFN8SzXZjw5eWb5e5n0spX6fZYI14tq/ytXEn7gCi18i3hxf2
P/39KlnDVDD2kSoGGCxREDd8fI8Tsytg0N2VjTUqK8IuOygcb+SchYLZRZ/GP3SL
M7NEkOECgYEA3DsRqdsfMoILKVmDYIUm9VWX25uvFIpLr0vb0mkZZ8YBNIsxk9kU
Bvvy0v0txUqkVbwruUOz6RKlwN6ERMDuo6DSqjrknLV5NJiomXR3WIzsmPXtPHCP
M7jpPA6Pj7vp+mwLZP14h4pVXnEiseI/TGwbUUyJwh7akMMjEM5s4EcCgYEAxkUd
pHruG3SOFLopJ8Le1bdREOyAP8sWMKcZqwffb2Ph7QL8gd4jS2JS2A2H2T85f5QX
TmMAaq1ybtzydNFVlzzc/wYtf1AEuebRwXGJ+QeoTyQPgm2mltqGsNxA89iaUu5g
9NjG231xSV1TFUS1tM8TcE1BGlWX/slH09/KBLkCgYAQKd2wA78Ua+r5y/ISgXNM
ik2s9o4xCb1oTj8CAGi27xdjjdvDqCkKIi++QIq9ci8Bg+AJpuhrcMztjOdUTHy3
UbRkfhINVlxg+dtV9/BSdXVf/Jy+1NwNxcKIf+EVgYs4r/leHiRcflCpr9AGEj7n
EwAmRWHO94i/GzNVpnDjPQKBgC9mjrOoAAzoTJ9/8h7jY8rMTMEzCWY281MmX+iX
tT84Lecv85ZIuT9ofk3Qzk2N4/0wNnIzEQJv/Q+sGvr6oO/LSTpfC7mIM9kN6c/J
iSbUQdTJ1jtkY5NlIxZdUdn6SsrQ2rUurjwDPXZArYBEMPzNFOSgU5QsKm5rSJTK
x81hAoGAJJCWmCyppVpyjjIouRNzvK2aNUYNCJ6zRZE/dYtIGt2JFaaRaiZThhfT
INpOEm3sy9CLLALihilxFUsGFe9JPxB3b9t0zflVznydcjGw3SQvovyh8JzvoPQ1
+rwiLr5qCglt4WuCZK+gL+RzmK1trA4/B65cn6xg9ghEEHDXUCs=
-----END RSA PRIVATE KEY-----
)EOF";

// The server's public certificate which must be shared
const char server_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIICuTCCAaECFA1QaDc3FpfpU9gLTat4GFW18ECrMA0GCSqGSIb3DQEBCwUAMBkx
FzAVBgNVBAMMDjE5Mi4xNjguMTMuMTAzMB4XDTI2MDMxNTEyMTMxNFoXDTI3MDMx
NTEyMTMxNFowGTEXMBUGA1UEAwwOMTkyLjE2OC4xMy4xMDYwggEiMA0GCSqGSIb3
DQEBAQUAA4IBDwAwggEKAoIBAQCqkSUVUtq8D/iWLKoAsDdWmKsGFt2Xj3fh3Cmx
p2tFvviVgwOYIWiueNzCiLwUFOabAvSa1wEDAUYsHOydKCWuruauYCp0osev19Ux
klIXM+rcz5MfAx8kPeR7gmzEu3iBsupQCHbfmTPZBzdFf1RxIJ7/zdu7xoTQ3arP
mwgzeuX0K3HsoTYDhz19OLjbf6iBPlH3yBIZmecs81a4t0qPY8yAPdvgQR0O4FRm
/EpQQPqpycn1UWkCs8G05JREzU0R/uAm46E1N10ATKaCa6HtVTgRcKszYu6Qwk+u
8xuqhgjN/qJdUNesrhT8DASuNQifM3WfrctyuNhT3yqxNS9PAgMBAAEwDQYJKoZI
hvcNAQELBQADggEBAMaPUWo3JNBGEKnp3CIMya1uqFSeAZ2WYXBRfmbHBb/R/NsU
aQ1LlA3DittkDfP4ny8Zh3X+r0R4ryK9tltrf3LlLDiTAqujj2rvyIA18RTT7YUk
bG2gVkBAgJLStaQPo3WuXlsljwSCCOSJ15juhbV1LSr3uDUAllfLqZk6mNcIwJoE
FPo5wLyGyNMlfO5Vr7a7Fq7/LgvT35c1mj2acqnsUDCll3vUjwZp1Y0vv9dUkeH7
Ib+fh12a6pcrPF05rIDu8ep8JDJYZJ75Tlosia3DFi4Ll+7u6gqmP0okXJ4hnrhj
K7AiCInPr6Y3970GGVL4Bwgy1yE7B+Yg9p+pWx0=
-----END CERTIFICATE-----

)EOF";

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
ModbusTLS mb;
// Set time via NTP, as required for x.509 validation
void setClock() {

  configTime(5 * 3600, 0, "ntp1.aliyun.com", "1.cn.pool.ntp.org");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }

  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}
bool c(IPAddress ip) {
  Serial.println(ip);
  return true;
}
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
  setClock();
  mb.server(MODBUSTLS_PORT, server_cert, server_private_key, ca_cert);

  mb.addHreg(REG_TEMPERATURE);
  mb.addHreg(REG_HUMIDITY);

  Serial.println("Modbus TLS服务器已启动");
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
    Serial.printf("Temperature: %.1f℃, Humidity: %.1f%%\n",
                  temperature / 10.0, humidity / 10.0);
  }
  mb.task();

  delay(10);
}