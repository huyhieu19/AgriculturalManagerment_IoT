/*
Board2: 1642BCDF-FEDC-4706-820A-20AA0AA4DF6C
ND-DA: D1, (đỏ nguôn, cam tín hiệu)
ND: 65bbe9da-7163-4689-b20b-9e21dabd4860
DA: 720b8be4-7064-43b9-8c6a-106f0accbfaa
Mưa: A0 (tím tín hiệu, nguồn xanh lá)
Mua: ddca1d26-577d-433c-bcf9-03c83cb18ac1
Relay: D3 : ECE53418-503E-4CD3-9BA5-96A0FFFE65B6

*/

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
// const char *ssid = "HuyHieu";
// const char *password = "20192855";
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
const char *moduleUrl = "1642BCDF-FEDC-4706-820A-20AA0AA4DF6C";

// const char *ND = "6EA49BEF-B141-4567-B43A-CE4FBF1AD348";
// const char *DA = "CBBB90A0-14B2-47D1-9EE1-10934185B8AA";
// const char *MUA = "6239FA8A-129B-46C1-A3CB-328AC318EA07";
// const char *AD = "0AE1FF7F-450A-4A90-853E-3ED64F2899A4";

const char *ND = "65bbe9da-7163-4689-b20b-9e21dabd4860";
const char *DA = "720b8be4-7064-43b9-8c6a-106f0accbfaa";
const char *MUA = "ddca1d26-577d-433c-bcf9-03c83cb18ac1";

const char *DEN = "ECE53418-503E-4CD3-9BA5-96A0FFFE65B6";

const char *idD8 = "A529949C-252D-42A7-B7EA-4359DFC492B3";
const char *Async = "async";

// GPIO define gate name

const uint8_t gateMeasure1 = D1; // nhiet do do am

const uint8_t gateControlD3 = D3;

// Initialize DHT to measure temperature and humidity
DHT dht1(gateMeasure1, DHT11);

// Initialize client and wifi
WiFiClient espClient;
PubSubClient client(espClient);

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
}

// reading status gates control
void readControl()
{
  // Read the status of the GPIOs
  int gateControl1Status = digitalRead(gateControlD3);
  // Print
  Serial.print("GPIO 6 :");
  Serial.println(gateControl1Status);
  Serial.print("GPIO 7 :");
  String payloadControl1 = String(gateControl1Status);
  std::string topicControl1 = std::string(systemUrl) + "/w/" + std::string("") + "/res";
  std::string topicControl2 = std::string(systemUrl) + "/w/" + std::string("") + "/res";

  // Send to broker
  client.publish(topicControl1.c_str(), payloadControl1.c_str());
}

// reading status gates measure
void readDHT11()
{
  int h1 = round(dht1.readHumidity());
  int t1 = round(dht1.readTemperature());
  int mua = round(analogRead(A0));
  if (mua < 800)
  {
    mua = 1;
  }
  else if (mua > 800)
  {
    mua = 0;
  }
  Serial.println(mua);
  Serial.println(t1);
  Serial.println(h1);

  // Kích thước bộ nhớ được cấp phát cho đối tượng JSON (tùy thuộc vào dự án của bạn)
  const size_t capacity = JSON_OBJECT_SIZE(10);
  DynamicJsonDocument RES1(capacity);
  DynamicJsonDocument RES2(capacity);
  RES1[ND] = String(t1);
  RES1[DA] = String(h1);
  RES1[MUA] = String(mua);

  String jsonString1;
  serializeJson(RES1, jsonString1);
  std::string topic = std::string(systemUrl) + "/r/" + std::string(moduleUrl);
  client.publish((topic).c_str(), jsonString1.c_str());

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

// void ControlBarie(int a)
// {
//   int pos = 0;
//   if (a == 1)
//   {
//     for (pos = 0; pos < 95; pos += 1)
//     {
//       myservo.write(pos);
//       delay(15);
//     }
//   }
//   if (a == 0)
//   {
//     for (pos = 95; pos >= 1; pos -= 1)
//     {
//       myservo.write(pos);
//       delay(15);
//     }
//   }
// }
/*This function is used to control the state of pinmodes
when receiving control from the topic and control payload*/
void controlDeviceByTopic(String topicString, String payload)
{
  if (topicString == String(DEN))
  {
    if (payload == "1")
    {
      digitalWrite(gateControlD3, HIGH);
      client.publish("3c531531-d5f5-4fe3-9954-5afd76ff2151/w/ECE53418-503E-4CD3-9BA5-96A0FFFE65B6/c", "c");
    }
    else if (payload == "0")
    {
      digitalWrite(gateControlD3, LOW);
      client.publish("3c531531-d5f5-4fe3-9954-5afd76ff2151/w/ECE53418-503E-4CD3-9BA5-96A0FFFE65B6/c", "c");
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
  if (topicArray[1] == "w")
  {
    controlDeviceByTopic(topicArray[2], payloadString);
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

  pinMode(gateControlD3, OUTPUT);
  digitalWrite(gateControlD3, LOW);

  pinMode(gateMeasure1, INPUT);

  // pinMode(gateControlD3, OUTPUT);
  // myservo.attach(gateControlD3);
  // myservo.write(0);

  dht1.begin();
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