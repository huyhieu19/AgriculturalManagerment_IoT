// your_implementation.cpp
#include "Function.h"

const char *mqttUser = "your_MQTT_username";
const char *mqttPassword = "your_MQTT_password";
void reconnect()
{
    while (!client.connected())
    {
        Serial.println("Attempting MQTT connection...");
        if (client.connect("ESP8266Client", mqttUser, mqttPassword))
        {
            Serial.println("Connected to MQTT broker");
            client.subscribe("your_topic");
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

void readControl()
{
    // Đọc trạng thái của các GPIO
    Serial.print("D6:");
    Serial.println(digitalRead(G6));

    Serial.print("D7 1 :");
    Serial.println(digitalRead(G7));

    // gửi lên broker
    String G6Value = String(digitalRead(G6));
    String G7Value = String(digitalRead(G7));

    std::string topicD6 = std::string(systemUrl) + "/" + std::string(IdD6);
    std::string topicD7 = std::string(systemUrl) + "/" + std::string(IdD7);
    client.publish((topicD6 + "/" + TypeW + "/" + nameRefD6).c_str(), G6Value.c_str());
    client.publish((topicD7 + "/" + TypeW + "/" + nameRefD7).c_str(), G7Value.c_str());
}

void readDHT11_1(char *Id)
{
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    std::string topic = std::string(systemUrl) + "/" + std::string(Id);
    client.publish((topic + "/" + TypeR + "/ND").c_str(), String(t).c_str());
    client.publish((topic + "/" + TypeR + "/DA").c_str(), String(h).c_str());

    Serial.println("Published temperature: " + String(t));
    Serial.print("Published humidity: " + String(h));
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
    String topicStr = String(topic);
    String payloadStr = "";
    for (uint i = 0; i < length; i++)
    {
        payloadStr += (char)payload[i];
    }

    Serial.print("Received message from topic: " + topicStr);
    Serial.print("Payload: " + payloadStr);
    // tách topic
    const int MAX_TOPICS = 5;
    String topicArray[MAX_TOPICS];
    splitTopic(topicStr, topicArray, MAX_TOPICS);

    // từ topic và lệnh thực hiện bật/tắt

    if (topicArray[3] == nameRefD6 && topicArray[1] == IdD6)
    {
        if (IdD7 == "1")
        {
            digitalWrite(G6, HIGH);
        }
        else
        {
            digitalWrite(G6, LOW);
        }
    }
    if (topicArray[3] == nameRefD7 && topicArray[1] == IdD7)
    {
        if (IdD7 == "1")
        {
            digitalWrite(G7, HIGH);
        }
        else
        {
            digitalWrite(G7, LOW);
        }
    }
}
