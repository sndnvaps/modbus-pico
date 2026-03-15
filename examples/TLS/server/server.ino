/*
  Modbus-Uni - Most complete Modbus Library for Arduino

  Modbus/TCP Security Server for ESP8266 Example
  
  (c)2020 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
  This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/

#include <WiFi.h>
#include <time.h>
#include <ModbusTLS.h>

// The hardcoded certificate authority for this example.
// Don't use it on your own apps!!!!!

const char ca_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIClDCCAf2gAwIBAgIUFMKCrXqL9rRWIB7qbl1OFUEZfLcwDQYJKoZIhvcNAQEL
BQAwXDELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoM
GEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDEVMBMGA1UEAwwMc25kbnZhcHMuY29t
MB4XDTI2MDMxNTA0MzQxN1oXDTI2MDQxNDA0MzQxN1owXDELMAkGA1UEBhMCQVUx
EzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoMGEludGVybmV0IFdpZGdpdHMg
UHR5IEx0ZDEVMBMGA1UEAwwMc25kbnZhcHMuY29tMIGfMA0GCSqGSIb3DQEBAQUA
A4GNADCBiQKBgQDden+490CN2DBk5RvtKO2oee5d37g3kV6J6cMxK4YiPHsXL5ZQ
b/lnguOQ27NcP37dXoX1ZPgsVCAnIn3nRg83Cty1Nwng0+rs+ntrjxUPrsx4hRZ8
CB3stLIyCgtpOXsZiKOMJWJxoLAJ+nwx0ZOvX7RdsHnw8gXKTZWvwxnSDQIDAQAB
o1MwUTAdBgNVHQ4EFgQUz3iz62MNmQa1+rDx1YEjekWyTLwwHwYDVR0jBBgwFoAU
z3iz62MNmQa1+rDx1YEjekWyTLwwDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0B
AQsFAAOBgQC1eiJavpJW4dUrCsVxK6itDih5VMYSNBPteFljoZwGeYYILkA67gDu
A6pgUr+lNFvIsrfzzxL9jEGGd8JCo3+MRYxICZI2yk3T35Kgf7imSuOByTBSNVaP
nm9yx7XaZwvgsK3zQqtcYncQGrqeAeqMzGiibJ5C76RhyJZTKE3tcA==
-----END CERTIFICATE-----
)EOF";

// The server's private key which must be kept secret
const char server_private_key[] PROGMEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIICXAIBAAKBgQDcJp0feM9gUhK1U6p118ceIHyf1Bz6KXO6IS2b6jTnWN/vFjyJ
v2jcUxO2iYS89BD9yJ5N3gRt6CanBZan7SL0kmGdcv/yScs5Jtonjo4lAXkmKA9n
E2KHf44eYGMjCBS6tHJ6PWLf3bLVXveHxypoyLG9JaUs8zs1h/9lOGyX0QIDAQAB
AoGAQnYhzyOAr5p3bWhxuJvI0A6MsQ+vI9KpzlI/26cMC5+oExzKb+dqN8GY3O2N
NiIUkxkiuW1CNw/zw06LmeycNVw2cp1PYkvPtqZ/jI1pV1Y7QtIgRfl8wSuc8xSZ
8Wkx19SJFRHOnDUNxatg8CvHKVFP6Z964XOtN+l/CvXzTPECQQDwGgDguapuD0Ww
vKVjN/0VOHP9R2WCDi+oYw4DSjZY+isjrPdLFgw+PzQ8/NmX5VmioMobv4N8Q6sh
ouu4QfyFAkEA6rpsJLtvtU4Q9bSBzrhKD802v79zu4BhGaKCOHdrKXCKb83I4jvg
UgmO7OvLGCFfNa5Dg8bheIlvKq/L5ZsF3QJAaFfUo9TsNRJcBfelpF3TlaJH/f1G
JuQFYBdUIqfgwIBPuRxVCAOX5IFRMWPtKC7a6msYCkELYjiCKYcFC7lZhQJBAJpn
QtfuzA6xaYqW0ISQyXcXgJolcBW43yajtZE7TKyXsRjWfvwCcw7D5taGTLR0z1Ja
bWLzokFN9mhX94HztakCQGtxWzVf9U0au9mQw3P4wzfYvlwMQno7pwyMQ0+ZPwVN
wcYdWgEecwkZP+AlEX4qeVqBm/vQM+/QQq8g3wLdu+Y=
-----END RSA PRIVATE KEY-----
)EOF";

// The server's public certificate which must be shared
const char server_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIICqTCCAhKgAwIBAgIBGDANBgkqhkiG9w0BAQsFADBcMQswCQYDVQQGEwJBVTET
MBEGA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0cyBQ
dHkgTHRkMRUwEwYDVQQDDAxzbmRudmFwcy5jb20wHhcNMjYwMzE1MDQzNjEzWhcN
MjcwMzE1MDQzNjEzWjBcMQswCQYDVQQGEwJBVTETMBEGA1UECAwKU29tZS1TdGF0
ZTEhMB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMRUwEwYDVQQDDAxz
bmRudmFwcy5jb20wgZ8wDQYJKoZIhvcNAQEBBQADgY0AMIGJAoGBANwmnR94z2BS
ErVTqnXXxx4gfJ/UHPopc7ohLZvqNOdY3+8WPIm/aNxTE7aJhLz0EP3Ink3eBG3o
JqcFlqftIvSSYZ1y//JJyzkm2ieOjiUBeSYoD2cTYod/jh5gYyMIFLq0cno9Yt/d
stVe94fHKmjIsb0lpSzzOzWH/2U4bJfRAgMBAAGjezB5MAkGA1UdEwQCMAAwLAYJ
YIZIAYb4QgENBB8WHU9wZW5TU0wgR2VuZXJhdGVkIENlcnRpZmljYXRlMB0GA1Ud
DgQWBBTxZ6Ao+tEoQSri6fmY6HPgjldcyDAfBgNVHSMEGDAWgBTPeLPrYw2ZBrX6
sPHVgSN6RbJMvDANBgkqhkiG9w0BAQsFAAOBgQBGP9+/WxKd+stCqU+BD9R9bf9m
v7PnPRABSB4mbtKKBWRXur1dtTMQmKVmxFO32EJRCg6CObNf+gMpONY3AnC0gHJa
7iF/DRW8xPnbn/p/5PspR6kMeYrBJK2gpoXjnWQGyz3n3DCxSIiToMJ/qpAFKQlN
RgRmkV18n2/t5yaDqw==
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