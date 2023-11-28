#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <string>
#include <PubSubClient.h>
#include "DHT.h"
#include <Wire.h>

// Thông tin Wi-Fi
const char *ssid = "FPT Telecom.2.4G";
const char *password = "22121999";

// khởi tạo dht 11
const int GATEDHT1 = D1;
DHT dht1(GATEDHT1, DHT11);
const int GATEDHT2 = D2;
DHT dht2(GATEDHT2, DHT11);

// khơi tạo client và wifi
WiFiClient espClient;
PubSubClient client(espClient);

// Thông tin MQTT broker
const char *mqttServer = "broker.emqx.io";
const int mqttPort = 1883;
const char *mqttUser = "your_MQTT_username";
const char *mqttPassword = "your_MQTT_password";
const char *systemUrl = "3c531531-d5f5-4fe3-9954-5afd76ff2151";
const char *idD1 = "ff824d3a-2548-4f16-b111-d102d3a3cdb4";
const char *idD2 = "91C90B9F-41E0-4777-8397-82BF44C9FA23";
const char *idD6 = "5DF0F490-73B9-4757-8A01-789874D0F810";
// // thông tin topic input out put
// const char *humidity = "humidity";
// const char *temperature = "temperature";

// GPIO
const int gpioControl0 = D6;
const int gpioControl1 = D5;
const int gpioControl2 = D2;
const int gpioControl3 = D3;

const int NDDA_1 = D1;
const int NDDA_2 = D2;

void reconnect()
{
  while (!client.connected())
  {
    Serial.println("Attempting MQTT connection...");

    if (client.connect("ESP8266Client", mqttUser, mqttPassword))
    {
      // Đăng ký theo dõi các topic
      std::string topic = std::string(systemUrl) + "/" + std::string(idD6) + "/W";
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

void readControl()
{

  // Đọc trạng thái của các GPIO
  Serial.print("GPIO 6 :");
  Serial.println(digitalRead(gpioControl0));

  Serial.print("GPIO 5 :");
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

  std::string topic = std::string(systemUrl) + "/" + std::string(idD1);
  client.publish((topic + "/W" + "/IsOnWater").c_str(), water.c_str());
  client.publish((topic + "/W" + "/IsOnFan").c_str(), fan.c_str());
  client.publish((topic + "/W" + "/IsOnLamp").c_str(), lamp.c_str());
  client.publish((topic + "/W" + "/IsOnCover").c_str(), cover.c_str());
}

void readDHT11()
{

  float h1 = dht1.readHumidity();
  float t1 = dht1.readTemperature();

  String temperaturePayload = String(t1);
  String humidityPayload = String(h1);

  std::string topic = std::string(systemUrl) + "/" + std::string(idD1);
  client.publish((topic + "/R" + "/ND").c_str(), temperaturePayload.c_str());
  client.publish((topic + "/R" + "/DA").c_str(), humidityPayload.c_str());

  Serial.print("Published temperature: ");
  Serial.println(temperaturePayload);
  Serial.print("Published humidity: ");
  Serial.println(humidityPayload);

  float h2 = dht2.readHumidity();
  float t2 = dht2.readTemperature();

  temperaturePayload = String(t2);
  humidityPayload = String(h2);

  topic = std::string(systemUrl) + "/" + std::string(idD2);
  client.publish((topic + "/R" + "/ND").c_str(), temperaturePayload.c_str());
  client.publish((topic + "/R" + "/DA").c_str(), humidityPayload.c_str());

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

  if (topicArray[1] == String(idD6))
  {
    if (payloadString == "6")
    {
      digitalWrite(gpioControl0, HIGH);
    }
    else
    {
      digitalWrite(gpioControl0, LOW);
    }
  }
  if (topicArray[1] == String(idD6))
  {
    if (payloadString == "5")
    {
      digitalWrite(gpioControl1, HIGH);
    }
    else
    {
      digitalWrite(gpioControl1, LOW);
    }
  }
  // if (topicArray[4] == "IsOnLamp")
  // {
  //   if (payloadString == "1")
  //   {
  //     digitalWrite(gpioControl2, HIGH);
  //   }
  //   else
  //   {
  //     digitalWrite(gpioControl2, LOW);
  //   }
  // }
  // if (topicArray[4] == "IsOnCover")
  // {
  //   if (payloadString == "1")
  //   {
  //     digitalWrite(gpioControl3, HIGH);
  //   }
  //   else
  //   {
  //     digitalWrite(gpioControl3, LOW);
  //   }
  // }
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
      std::string topic = std::string(systemUrl) + "/" + std::string(idD6) + "/W";
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
  pinMode(GATEDHT1, INPUT);
  pinMode(GATEDHT2, INPUT);

  digitalWrite(gpioControl0, LOW);
  digitalWrite(gpioControl1, LOW);
  digitalWrite(gpioControl2, LOW);
  digitalWrite(gpioControl3, LOW);

  dht1.begin();
  dht2.begin();
}

void loop()
{

  static unsigned long lastLoop1Time = 0;   // Lưu thời điểm thực hiện vòng lặp 1
  static unsigned long lastLoop2Time = 0;   // Lưu thời điểm thực hiện vòng lặp 2
  const unsigned long loop1Interval = 5000; // Thời gian giữa các lần chạy vòng lặp 1 (5s)
  const unsigned long loop2Interval = 500;  // Thời gian giữa các lần chạy vòng lặp 2 (3s)

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

    // delay(7000);
    //  Cập nhật thời điểm thực hiện vòng lặp 1
    lastLoop1Time = millis();
  }

  // Vòng lặp 2 (chạy mỗi 0.5 giây)
  if (millis() - lastLoop2Time >= loop2Interval)
  {
    // Thực hiện các hành động của vòng lặp 2
    // ...

    client.loop();
    // Cập nhật thời điểm thực hiện vòng lặp 2
    lastLoop2Time = millis();
  }

  // Các hành động khác trong vòng lặp chính
  // ...
}