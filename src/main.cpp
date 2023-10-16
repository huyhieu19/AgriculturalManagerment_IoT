#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <string>
#include <PubSubClient.h>
#include "DHT.h"
#include <Wire.h>

// Thông tin Wi-Fi
const char *ssid = "DatBeoDz";
const char *password = "25312001";

// khởi tạo dht 11
const int DHTPIN = D5;
DHT dht(DHTPIN, DHT11);

// khơi tạo client và wifi
WiFiClient espClient;
PubSubClient client(espClient);

// Thông tin MQTT broker
const char *mqttServer = "broker.emqx.io";
const int mqttPort = 1883;
const char *mqttUser = "your_MQTT_username";
const char *mqttPassword = "your_MQTT_password";
const char *systemUrl = "d6rjcudf7yfrokfyd6w84or994kffef";
const char *deviceId = "thisisdeviceid1";

// thông tin topic input out put
const char *humidity = "humidity";
const char *temperature = "temperature";

// GPIO
const int gpioControl0 = D0;
const int gpioControl1 = D1;
const int gpioControl2 = D2;
const int gpioControl3 = D3;
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

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Đọc trạng thái của các GPIO
  Serial.print("GPIO 0 :");
  Serial.println(digitalRead(gpioControl0));

  Serial.print("GPIO 1 :");
  Serial.println(digitalRead(gpioControl1));

  Serial.print("GPIO 2 :");
  Serial.println(digitalRead(gpioControl2));

  Serial.print("GPIO 3 :");
  Serial.println(digitalRead(gpioControl3));

  // gửi lên broker
  String water = String(digitalRead(gpioControl0));
  String fan = String(digitalRead(gpioControl1));
  String lamp = String(digitalRead(gpioControl2));
  String cover = String(digitalRead(gpioControl3));

  std::string topic = std::string(systemUrl) + "/" + std::string(deviceId);
  client.publish((topic + "/W" + "/IsOnWater").c_str(), water.c_str());
  client.publish((topic + "/W" + "/IsOnFan").c_str(), fan.c_str());
  client.publish((topic + "/W" + "/IsOnLamp").c_str(), lamp.c_str());
  client.publish((topic + "/W" + "/IsOnCover").c_str(), cover.c_str());
}

void readDHT11()
{

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  String temperaturePayload = String(t);
  String humidityPayload = String(h);

  std::string topic = std::string(systemUrl) + "/" + std::string(deviceId);
  client.publish((topic + "/R" + "/temperature").c_str(), temperaturePayload.c_str());
  client.publish((topic + "/R" + "/humidity").c_str(), humidityPayload.c_str());

  Serial.print("Published temperature: ");
  Serial.println(temperaturePayload);
  Serial.print("Published humidity: ");
  Serial.println(humidityPayload);
}
void splitTopic(String topic, String *topicArray, int arraySize)
{
  int lastIndex = 0;
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
  String topicString = String(topic);
  String payloadString = "";
  for (int i = 0; i < length; i++)
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

  if (topicArray[4] == "IsOnWater")
  {
    if (payloadString == "1")
    {
      digitalWrite(gpioControl0, HIGH);
    }
    else
    {
      digitalWrite(gpioControl0, LOW);
    }
  }
  if (topicArray[4] == "IsOnFan")
  {
    if (payloadString == "1")
    {
      digitalWrite(gpioControl1, HIGH);
    }
    else
    {
      digitalWrite(gpioControl1, LOW);
    }
  }
  if (topicArray[4] == "IsOnLamp")
  {
    if (payloadString == "1")
    {
      digitalWrite(gpioControl2, HIGH);
    }
    else
    {
      digitalWrite(gpioControl2, LOW);
    }
  }
  if (topicArray[4] == "IsOnCover")
  {
    if (payloadString == "1")
    {
      digitalWrite(gpioControl3, HIGH);
    }
    else
    {
      digitalWrite(gpioControl3, LOW);
    }
  }
}

void setup()
{
  Serial.begin(115200);

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
      std::string topic = std::string(systemUrl) + "/" + std::string(deviceId) + "/W" + "/L" + "/#";
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
  pinMode(gpioControl0, OUTPUT);
  pinMode(gpioControl1, OUTPUT);
  pinMode(gpioControl2, OUTPUT);
  pinMode(gpioControl3, OUTPUT);
  pinMode(DHTPIN, INPUT);

  digitalWrite(gpioControl0, LOW);
  digitalWrite(gpioControl1, LOW);
  digitalWrite(gpioControl2, LOW);
  digitalWrite(gpioControl3, LOW);

  dht.begin();
}

void loop()
{
  // kết nối broker
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  // gửi tín hiệu
  readControl();
  readDHT11();

  delay(2000);
}
