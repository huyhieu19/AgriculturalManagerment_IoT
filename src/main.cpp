#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <string>
#include <PubSubClient.h>
#include "DHT.h"
#include <Wire.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

// Wi-Fi information
// const char *ssid = "BKSTAR";
// const char *password = "stemstar";
const char *ssid = "FPT Telecom.2.4G";
const char *password = "22121999";

// MQTT broker information
// const char *mqttServer = "broker.emqx.io";
const char *mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char *mqttUser = "your_MQTT_username";
const char *mqttPassword = "your_MQTT_password";

// Thông tin Topic
// --- Topic: SystemUrl/idgate/type/name
const char *systemUrl = "3c531531-d5f5-4fe3-9954-5afd76ff2151";
const char *moduleUrl = "66DB8F87-1A1E-4701-F2EE-08DBF79AE6C9";
const char *idD1 = "664CB71A-D3F6-4FBF-95DD-490CF746108B";
const char *idD2 = "91C90B9F-41E0-4777-8397-82BF44C9FA23";

const char *idD5 = "91C90B9F-41E0-4777-8397-82BF44C9FA23";
const char *idD6 = "A61D10C0-B931-4906-98F4-18F8FEF95EB1";
const char *idD7 = "55E05E4F-575C-4656-8099-50ABCFEE7DE8";
const char *idD8 = "A529949C-252D-42A7-B7EA-4359DFC492B3";
const char *Async = "async";

// GPIO define gate name
const uint8_t gateControl1 = D6;
const uint8_t gateControl2 = D7;
const uint8_t gateControl3 = D8;

const uint8_t gateMeasure1 = D1;
const uint8_t gateMeasure2 = D2;
const uint8_t gateMeasure3 = D3;
const uint8_t gateMeasure4 = D4;

// Initialize DHT to measure temperature and humidity
DHT dht1(gateMeasure2, DHT11);
DHT dht2(gateMeasure2, DHT11);

// Initialize client and wifi
WiFiClient espClient;
PubSubClient client(espClient);

// Started SoftwareSerial at RX and TX pin of ESP8266/NodeMCU
SoftwareSerial SoftSerial(3, 1);

/*---------Begin Working with EEPROM---------------*/

/*---------End Working with EEPROM---------------*/

// The function tries to reconnect MQTT when the connection is lost
void reconnect()
{
  while (!client.connected())
  {
    Serial.println("Attempting MQTT connection...");

    if (client.connect("ESP8266Client", mqttUser, mqttPassword))
    {
      // Đăng ký theo dõi các topic
      std::string topic = std::string(systemUrl) + "/" + std::string(moduleUrl) + "/#";
      client.subscribe(topic.c_str());
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

// reading status gates control
void readControl()
{
  // Read the status of the GPIOs
  int gateControl1Status = digitalRead(gateControl1);
  int gateControl2Status = digitalRead(gateControl2);
  // Print
  Serial.print("GPIO 6 :");
  Serial.println(gateControl1Status);
  Serial.print("GPIO 7 :");
  Serial.println(gateControl2Status);
  String payloadControl1 = String(gateControl1Status);
  String payloadControl2 = String(gateControl2Status);
  std::string topicControl1 = std::string(systemUrl) + "/w/" + std::string(idD6) + "/res";
  std::string topicControl2 = std::string(systemUrl) + "/w/" + std::string(idD7) + "/res";

  // Send to broker
  client.publish(topicControl1.c_str(), payloadControl1.c_str());
  client.publish(topicControl2.c_str(), payloadControl2.c_str());
}

// reading status gates measure
void readDHT11()
{
  int h1 = round(dht1.readHumidity());
  int t1 = round(dht1.readTemperature());
  int mua = digitalRead(gateMeasure1);
  int amdat = round(analogRead(A0));

  Serial.println(mua);
  Serial.println(amdat);
  Serial.println(t1);
  Serial.println(h1);

  // Kích thước bộ nhớ được cấp phát cho đối tượng JSON (tùy thuộc vào dự án của bạn)
  const size_t capacity = JSON_OBJECT_SIZE(10);
  DynamicJsonDocument ND_DA_D1(capacity);
  ND_DA_D1["6EA49BEF-B141-4567-B43A-CE4FBF1AD348"] = String(t1);
  ND_DA_D1["CBBB90A0-14B2-47D1-9EE1-10934185B8AA"] = String(h1);
  ND_DA_D1["0AE1FF7F-450A-4A90-853E-3ED64F2899A4"] = String(amdat);
  ND_DA_D1["6239FA8A-129B-46C1-A3CB-328AC318EA07"] = String(mua);

  String jsonString;
  serializeJson(ND_DA_D1, jsonString);
  std::string topic = std::string(systemUrl) + "/r/";
  // std::string topic = std::string(systemUrl) + "/r/" + std::string(idD1);
  client.publish((topic + "/ND_DA").c_str(), jsonString.c_str());
  SoftSerial.println(jsonString);

  Serial.print("Published temperature: ");
  Serial.println(String(t1));
  Serial.print("Published humidity: ");
  Serial.println(String(h1));
}

void splitTopic(String topic, String *topicArray, int arraySize)
{
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

void ControlDevice(uint8_t gateControl, int mode)
{
  digitalWrite(gateControl, mode);
}

/*This function is used to control the state of pinmodes
when receiving control from the topic and control payload*/
void controlDeviceByTopic(String topicString, String payload)
{
  if (topicString == String(idD6))
  {
    if (payload == "1")
    {
      ControlDevice(gateControl1, HIGH);
      Serial.print("Open 0");
      client.publish("3c531531-d5f5-4fe3-9954-5afd76ff2151/w/A61D10C0-B931-4906-98F4-18F8FEF95EB1/c", "c");
    }
    else if (payload == "0")
    {
      ControlDevice(gateControl1, LOW);
      Serial.print("Close 0");
      client.publish("3c531531-d5f5-4fe3-9954-5afd76ff2151/w/A61D10C0-B931-4906-98F4-18F8FEF95EB1/c", "c");
    }
  }
  if (topicString == String(idD7))
  {
    if (payload == "1")
    {
      digitalWrite(gateControl2, HIGH);
      Serial.print("Open 1");
      client.publish("3c531531-d5f5-4fe3-9954-5afd76ff2151/w/55E05E4F-575C-4656-8099-50ABCFEE7DE8/c", "c");
    }
    else if (payload == "0")
    {
      ControlDevice(gateControl2, LOW);
      Serial.print("Close 1");
      client.publish("3c531531-d5f5-4fe3-9954-5afd76ff2151/w/55E05E4F-575C-4656-8099-50ABCFEE7DE8/c", "c");
    }
  }
  if (topicString == String(idD8))
  {
    if (payload == "1")
    {
      digitalWrite(gateControl3, HIGH);
      client.publish("3c531531-d5f5-4fe3-9954-5afd76ff2151/w/A529949C-252D-42A7-B7EA-4359DFC492B3/c", "c");
    }
    else if (payload == "0")
    {
      ControlDevice(gateControl3, LOW);
      client.publish("3c531531-d5f5-4fe3-9954-5afd76ff2151/w/A529949C-252D-42A7-B7EA-4359DFC492B3/c", "c");
    }
  }
}

// Callback method
void callback(char *topic, byte *payload, unsigned int length)
{
  String topicString = String(topic);
  String payloadString = "";
  for (uint i = 0; i < length; i++)
  {
    payloadString += (char)payload[i];
  }

  Serial.print("Received message from topic: ");
  Serial.println(topicString);
  Serial.print("Payload: ");
  Serial.println(payloadString);

  // tách topic
  const int MAX_TOPICS = 5;
  String topicArray[MAX_TOPICS];
  splitTopic(topicString, topicArray, MAX_TOPICS);

  // từ topic và lệnh thực hiện bật/tắt
  if (topicArray[2] == "w")
  {
    controlDeviceByTopic(topicArray[3], payloadString);
  }
}

// setup method - (main method)
void setup()
{
  Serial.begin(9600);

  // Thiết lập kết nối Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Thiết lập kết nối MQTT
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected())
  {
    Serial.println("Connecting to MQTT broker...");

    if (client.connect("ESP8266Clien123232gfhds"))
    {
      Serial.println("Connected to MQTT broker");

      // Đăng ký theo dõi các topic
      std::string topic = std::string(moduleUrl) + "/#";
      client.subscribe(topic.c_str());
    }
    else
    {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(1000);
    }
  }

  pinMode(gateControl1, OUTPUT);
  pinMode(gateControl2, OUTPUT);
  pinMode(gateControl3, OUTPUT);
  pinMode(gateMeasure1, INPUT);
  pinMode(gateMeasure2, INPUT);
  pinMode(gateMeasure3, INPUT);
  pinMode(gateMeasure4, INPUT);

  digitalWrite(gateControl1, LOW);
  digitalWrite(gateControl2, LOW);

  dht1.begin();
  dht2.begin();
}

void loop()
{
  static unsigned long lastLoop1Time = 0;   // Lưu thời điểm thực hiện vòng lặp 1
  static unsigned long lastLoop2Time = 0;   // Lưu thời điểm thực hiện vòng lặp 2
  const unsigned long loop1Interval = 5000; // Thời gian giữa các lần chạy vòng lặp 1 (5s)
  const unsigned long loop2Interval = 100;  // Thời gian giữa các lần chạy vòng lặp 2 (0.1s)
  if (millis() - lastLoop1Time >= loop1Interval)
  {
    if (!client.connected())
    {
      reconnect();
    }
    // readControl();
    readDHT11();
    lastLoop1Time = millis();
  }
  if (millis() - lastLoop2Time >= loop2Interval)
  {
    client.loop();
    lastLoop2Time = millis();
  }
}