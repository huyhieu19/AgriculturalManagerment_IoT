// your_implementation.cpp
#include "Function.h"
#include <EEPROM.h>

// Thông tin Wifi
#define SSID "FPT Telecom.2.4G"
#define WIFIPW "22121999"
// Thông tin kết nối MQTT broker
const char *MQTTSV = "broker.emqx.io";
const int MQTTPORT = 1883;
const char *MQTTUSER = "MQTTNAME";
const char *MQTTPW = "MQTTPW";
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

const char *onDevice = "1";

// khơi tạo client và wifi
void ConnectWIFI()
{
    // Thiết lập kết nối Wi-Fi
    WiFi.begin(SSID, WIFIPW);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(5000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
}
void ConnectMQTT(PubSubClient client)
{
    // Thiết lập kết nối MQTT
    client.setServer(MQTTSV, MQTTPORT);
    client.setCallback(callback);

    while (!client.connected())
    {
        Serial.println("Connecting to MQTT broker...");

        if (client.connect("ESP8266Client"))
        {
            Serial.println("Connected to MQTT broker");
            // Đăng ký theo dõi các topic
            // client.subscribe(topicD6);
            // client.subscribe(topicD7);
            // client.subscribe(TopicThresholdW);
        }
        else
        {
            Serial.print("Failed, rc=");
            Serial.print(client.state());
            Serial.println(" Retrying in 5 seconds...");
            delay(5000);
        }
    }
}

void reconnect(PubSubClient client)
{
    while (!client.connected())
    {
        Serial.println("Attempting MQTT connection...");
        if (client.connect("ESP8266Client123", MQTTUSER, MQTTPW))
        {
            Serial.println("Connected to MQTT broker");
            // Đăng ký theo dõi các topic
            client.subscribe(topicD6);
            client.subscribe(topicD7);
        }
        else
        {
            Serial.print("Failed, rc=");
            Serial.print(client.state());
            Serial.println(" Retrying in 5 seconds...");
            delay(1000);
        }
    }
}

void readControl(PubSubClient client, uint8_t GateD6, uint8_t GateD7)
{
    // Đọc trạng thái của các GPIO
    Serial.println("D6:" + digitalRead(GateD6));
    Serial.println("D7:" + digitalRead(GateD7));

    // gửi lên broker
    client.publish(topicD6, String(digitalRead(GateD6)).c_str());
    client.publish(topicD7, String(digitalRead(GateD7)).c_str());
}

void readDHT11(PubSubClient client, DHT dht1, uint8_t GateDHT1, DHT dht2, uint8_t GateDHT2)
{
    int pin1State = digitalRead(GateDHT1);
    int pin2State = digitalRead(GateDHT2);

    if (pin1State == HIGH)
    {
        float h1 = dht1.readHumidity();
        float t1 = dht1.readTemperature();
        client.publish(topicD1_T, String(t1).c_str());
        client.publish(topicD1_H, String(h1).c_str());
        Serial.println("Published temperature D1: " + String(t1) + "\t" + "Published humidity D1: " + String(h1));
    }
    // if (pin2State == HIGH)
    // {
    //     float h2 = dht2.readHumidity();
    //     float t2 = dht2.readTemperature();
    //     client.publish(topicD2_T, String(t2).c_str());
    //     client.publish(topicD2_H, String(h2).c_str());
    //     Serial.println("Published temperature D2: " + String(t2) + "\t" + "Published humidity D2: " + String(h2));
    // }
}

void splitTopic(String topic, String *topicArray, int arraySize)
{
    // Topic: SystemId/DeviceId/Type/NRef
    uint lastIndex = 0;
    int index = topic.indexOf('/');
    int i = 0;

    while (index != -1 && i < arraySize)
    {
        topicArray[i] = topic.substring(lastIndex, index);
        lastIndex = index + 1;
        index = topic.indexOf('/', lastIndex);
        i++;
    }

    if (lastIndex < topic.length() && i < arraySize)
    {
        topicArray[i] = topic.substring(lastIndex);
    }
}

void callback(char *topic, byte *payload, unsigned int length)
{
    char *payloadStr = "";

    for (uint i = 0; i < length; i++)
    {
        payloadStr += ((char)payload[i]);
    }

    Serial.print("Received message from topic: " + String(topic));
    Serial.print("Payload: " + String(payloadStr));

    // tách topic
    // const int MAX_TOPICS = 5;
    // String topicArray[MAX_TOPICS];
    // splitTopic(topicStr, topicArray, MAX_TOPICS);

    // từ topic và lệnh thực hiện bật/tắt

    if (topic == topicD6)
    {
        payloadStr == onDevice ? digitalWrite(GATE6, HIGH) : digitalWrite(GATE6, LOW);
    }
    else if (topic == topicD7)
    {
        payloadStr == onDevice ? digitalWrite(GATE7, HIGH) : digitalWrite(GATE7, LOW);
    }
    else if (topic == TopicThresholdW)
    {
        WriteEEPROM(topic, payload, length);
    }
}
void StartDHT(DHT dht1, DHT dht2)
{
    dht1.begin();
    dht2.begin();
}

void WriteEEPROM(char *topic, byte *payload, unsigned int length)
{
    // tách topic
    const int MAX_TOPICS = 5;
    String topicArray[MAX_TOPICS];
    splitTopic(topic, topicArray, MAX_TOPICS);
    // Khai báo các biến hoặc cấu trúc dữ liệu cho từng khu vực
    struct
    {
        int value1;
        int value2;
        int value3;
    } area1Data = {1, 456, 789};
    struct
    {
        int value1;
        int value2;
        int value3;
    } area2Data = {1, 456, 789};
    struct
    {
        int value1;
        int value2;
        int value3;
    } area3Data = {1, 456, 789};
    struct
    {
        int value1;
        int value2;
        int value3;
    } area4Data = {1, 456, 789};

    // Write data to flash memory
    for (int i = 0; i < length; i++)
    {
        EEPROM.write(i, payload[i]);
    }

    // Bắt đầu và xác định kích thước cho EEPROM
    EEPROM.begin(AREA4_ADDRESS + AREA_SIZE);

    // Ghi dữ liệu vào Flash
    EEPROM.put(AREA1_ADDRESS, area1Data);
    EEPROM.put(AREA2_ADDRESS, area2Data);
    EEPROM.put(AREA2_ADDRESS, area3Data);
    EEPROM.put(AREA2_ADDRESS, area4Data);

    EEPROM.commit();
}
void ReadEEPROM(char *topic)
{
}