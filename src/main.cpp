#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <string>
#include <PubSubClient.h>
#include "DHT.h"
#include <Wire.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

// Wi-Fi information
const char *ssid = "FPT Telecom.2.4G";
const char *password = "22121999";

// MQTT broker information
const char *mqttServer = "broker.emqx.io";
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
const char *idD6 = "5DF0F490-73B9-4757-8A01-789874D0F810";
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

struct ThresholdValues
{
  float thresholdOpen = 0;
  float thresholdClose = 0;
};

const int NumThresholds = 3; // Số lượng giá trị ngưỡng
const int EEPROMAddress = 0;
ThresholdValues thresholds[NumThresholds];

// Initialize DHT to measure temperature and humidity
DHT dht1(gateMeasure1, DHT11);
DHT dht2(gateMeasure2, DHT11);

// Initialize client and wifi
WiFiClient espClient;
PubSubClient client(espClient);

/*---------Begin Working with EEPROM---------------*/

void readFromEEPROM()
{
  EEPROM.begin(sizeof(thresholds));
  EEPROM.get(EEPROMAddress, thresholds);
  EEPROM.end();
}

void updateThresholds(int index, float thresholdOpen, float thresholdClose)
{
  if (index >= 0 && index < NumThresholds)
  {
    thresholds[index].thresholdOpen = thresholdOpen;
    thresholds[index].thresholdClose = thresholdClose;

    EEPROM.begin(sizeof(thresholds));
    EEPROM.put(EEPROMAddress, thresholds);
    EEPROM.commit();
    EEPROM.end();
  }
}

void removeThreshold(int index)
{
  if (index >= 0 && index < NumThresholds)
  {
    thresholds[index].thresholdOpen = 0;
    thresholds[index].thresholdClose = 0;

    EEPROM.begin(sizeof(thresholds));
    EEPROM.put(EEPROMAddress, thresholds);
    EEPROM.commit();
    EEPROM.end();
  }
}

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
  // Kích thước bộ nhớ được cấp phát cho đối tượng JSON (tùy thuộc vào dự án của bạn)
  const size_t capacity = JSON_OBJECT_SIZE(4);
  DynamicJsonDocument ND_DA_D1(capacity);

  float h1 = dht1.readHumidity();
  float t1 = dht1.readTemperature();
  ND_DA_D1["ND"] = String(t1);
  ND_DA_D1["DA"] = String(h1);
  String jsonString;
  serializeJson(ND_DA_D1, jsonString);
  std::string topic = std::string(systemUrl) + "/r/" + std::string(idD1);
  client.publish((topic + "/ND_DA").c_str(), jsonString.c_str());
  Serial.print("Published temperature: ");
  Serial.println(String(t1));
  Serial.print("Published humidity: ");
  Serial.println(String(h1));

  if (thresholds[0].thresholdOpen > thresholds[0].thresholdClose)
  {
    if (t1 > thresholds[0].thresholdOpen)
    {
      client.publish((topic + "/auto/ND/open").c_str(), String(t1).c_str());
      Serial.print("Nhiet do -> open");
    }
    else if (t1 < thresholds[0].thresholdClose)
    {
      client.publish((topic + "/auto/ND/close").c_str(), String(t1).c_str());
      Serial.print("Nhiet do -> close");
    }
  }
  else
  {
    if (t1 < thresholds[0].thresholdOpen)
    {
      client.publish((topic + "/auto/DA/open").c_str(), String(t1).c_str());
      Serial.print("Nhiet do -> open");
    }
    else if (t1 > thresholds[0].thresholdClose)
    {
      client.publish((topic + "/auto/DA/close").c_str(), String(t1).c_str());
      Serial.print("Nhiet do -> close");
    }
  }

  if (thresholds[1].thresholdOpen > thresholds[1].thresholdClose)
  {
    if (h1 > thresholds[1].thresholdOpen)
    {
      client.publish((topic + "/auto/ND/open").c_str(), String(h1).c_str());
      Serial.print("Do am -> open");
    }
    else if (h1 < thresholds[1].thresholdClose)
    {
      client.publish((topic + "/auto/ND/close").c_str(), String(h1).c_str());
      Serial.print("Do am -> close");
    }
  }
  else
  {
    if (h1 < thresholds[1].thresholdOpen)
    {
      client.publish((topic + "/auto/DA/open").c_str(), String(h1).c_str());
      Serial.print("Do am -> open");
    }
    else if (h1 > thresholds[1].thresholdClose)
    {
      client.publish((topic + "/auto/DA/close").c_str(), String(h1).c_str());
      Serial.print("Do am -> close");
    }
  }
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
      client.publish("3c531531-d5f5-4fe3-9954-5afd76ff2151/w/5DF0F490-73B9-4757-8A01-789874D0F810/c", "c");
    }
    else if (payload == "0")
    {
      ControlDevice(gateControl1, LOW);
      client.publish("3c531531-d5f5-4fe3-9954-5afd76ff2151/w/5DF0F490-73B9-4757-8A01-789874D0F810/c", "c");
    }
  }
  if (topicString == String(idD7))
  {
    if (payload == "1")
    {
      digitalWrite(gateControl2, HIGH);
      client.publish("3c531531-d5f5-4fe3-9954-5afd76ff2151/w/55E05E4F-575C-4656-8099-50ABCFEE7DE8/c", "c");
    }
    else if (payload == "0")
    {
      ControlDevice(gateControl2, LOW);
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

  // tách payload
  String payloadArray[MAX_TOPICS];
  splitTopic(payloadString, payloadArray, MAX_TOPICS);

  // Topic: sys/module/idgate/type/
  if (topicArray[3] == "write")
  {
    updateThresholds(std::stoi(payloadArray[0].c_str()), std::atof(payloadArray[1].c_str()), std::atof(payloadArray[2].c_str()));
  }
}

// setup method - (main method)
void setup()
{
  Serial.begin(115200);

  // Reading EEPROM
  readFromEEPROM();

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

// loop method - (main method)
void loop()
{
  static unsigned long lastLoop1Time = 0;   // Lưu thời điểm thực hiện vòng lặp 1
  static unsigned long lastLoop2Time = 0;   // Lưu thời điểm thực hiện vòng lặp 2
  const unsigned long loop1Interval = 5000; // Thời gian giữa các lần chạy vòng lặp 1 (5s)
  const unsigned long loop2Interval = 100;  // Thời gian giữa các lần chạy vòng lặp 2 (0.1s)

  // Vòng lặp 1 (chạy mỗi 5 giây)
  if (millis() - lastLoop1Time >= loop1Interval)
  {
    // Thực hiện các hành động của vòng lặp 1
    // ...
    // kết nối broker
    if (!client.connected())
    {
      reconnect();
    }
    // gửi tín hiệu
    readControl();
    readDHT11();
    Serial.println(String(thresholds[0].thresholdClose) + "/" + String(thresholds[0].thresholdOpen));
    // delay(7000);
    //  Cập nhật thời điểm thực hiện vòng lặp 1
    lastLoop1Time = millis();
  }

  // Vòng lặp 2 (chạy mỗi 0.1 giây)
  if (millis() - lastLoop2Time >= loop2Interval)
  {
    // Thực hiện các hành động của vòng lặp 2
    // ...

    client.loop();

    // Cập nhật thời điểm thực hiện vòng lặp 2
    lastLoop2Time = millis();
  }
}