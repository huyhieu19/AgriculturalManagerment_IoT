
#include "Function.h"

// Thông tin Wifi
#define SSID "FPT Telecom.2.4G"
#define WIFIPW "22121999"

const u_int8_t GATE1 = D1;
const u_int8_t GATE2 = D2;
const u_int8_t GATE5 = D5;
const u_int8_t GATE6 = D6;
const u_int8_t GATE7 = D7;

// Định nghĩa kích thước của từng khu vực Flash
#define AREA_SIZE 512

// Định nghĩa địa chỉ bắt đầu của từng khu vực Flash
#define AREA1_ADDRESS 0
#define AREA2_ADDRESS (AREA1_ADDRESS + AREA_SIZE)
#define AREA3_ADDRESS (AREA2_ADDRESS + AREA_SIZE)
#define AREA4_ADDRESS (AREA3_ADDRESS + AREA_SIZE)
// Thông tin kết nối MQTT broker
char *MQTTSV = "broker.emqx.io";
uint16_t MQTTPORT = 1883;
char *MQTTUSER = "MQTTNAME";
char *MQTTPW = "MQTTPW";
const char *SYSURL = "3c531531-d5f5-4fe3-9954-5afd76ff2151";
const char *IDD1 = "ff824d3a-2548-4f16-b111-d102d3a3cdb4";
const char *NAME_D1_1 = "ND";
const char *NAME_D1_2 = "DA";
const char *IDD2 = "xxx";
const char *NAME_D2_1 = "ND";
const char *NAME_D2_2 = "DA";
const char *IDD5 = "xxx";
const char *NAME_D5_1 = "AD";
const char *IDD6 = "xxx";
const char *NAME_D6 = "O_F";
const char *IDD7 = "xxx";
const char *NAME_D7 = "O_F";

const char *TypeR = "R";
const char *TypeW = "W";

// define const variable
const char *topicD1_T = (std::string(SYSURL) + "/" + std::string(IDD1) + "/" + std::string(TypeR) + "/" + std::string(NAME_D1_1)).c_str();

const char *topicD1_H = (std::string(SYSURL) + "/" + std::string(IDD1) + "/" + std::string(TypeR) + "/" + std::string(NAME_D1_2)).c_str();
const char *topicD2_T = (std::string(SYSURL) + "/" + std::string(IDD2) + "/" + std::string(TypeR) + "/" + std::string(NAME_D2_1)).c_str();
const char *topicD2_H = (std::string(SYSURL) + "/" + std::string(IDD2) + "/" + std::string(TypeR) + "/" + std::string(NAME_D2_2)).c_str();

const char *topicD6 = (std::string(SYSURL) + "/" + std::string(IDD6) + "/" + std::string(TypeW) + "/" + std::string(NAME_D6)).c_str();
const char *topicD7 = (std::string(SYSURL) + "/" + std::string(IDD7) + "/" + std::string(TypeW) + "/" + std::string(NAME_D7)).c_str();

const char *TopicThresholdW = (std::string(SYSURL) + "/threshold/" + std::string(TypeW)).c_str();
const char *TopicThresholdR = (std::string(SYSURL) + "/threshold/" + std::string(TypeR)).c_str();

char *topics[2] = {
    "ac",
    "abc",
};
const char *onDevice = "1";

DHT dht1(GATE1, DHT11);
DHT dht2(GATE2, DHT11);

WiFiClient espClient;
PubSubClient client(espClient);

void setup()
{
  Serial.begin(115200);

  // Thiết lập kết nối Wi-Fi
  ConnectWIFI(SSID, WIFIPW);

  // Thiết lập kết nối MQTT

  ConnectMQTT(client, MQTTSV, MQTTPORT, topics);
  pinMode(GATE1, INPUT);
  pinMode(GATE2, INPUT);
  // pinMode(GATE5, INPUT);
  pinMode(GATE6, OUTPUT);
  pinMode(GATE6, INPUT);
  pinMode(GATE7, OUTPUT);
  pinMode(GATE7, INPUT);

  digitalWrite(GATE6, LOW);
  digitalWrite(GATE7, LOW);
  digitalWrite(GATE1, LOW);
  digitalWrite(GATE2, LOW);
  StartDHT(dht1, dht2);
}

void loop()
{
  // kết nối broker
  if (!client.connected())
  {
    reconnect(client, MQTTUSER, MQTTPW, topics);
    // ConnectMQTT(client, MQTTSV, MQTTPORT);
  }
  client.loop();
  // gửi tín hiệu
  // readControl(client, GATE6, GATE7);
  readDHT11(client, dht1, GATE1, topics, dht2, GATE2, topics);

  delay(5000);
}
