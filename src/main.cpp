
#include "Function.h"
const char *mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;

void setup()
{
  Serial.begin(115200);

  // Thiết lập kết nối Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(5000);
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
      std::string topicD6 = std::string(systemUrl) + "/" + std::string(IdD6) + "/" + TypeW + "/" + nameRefD6;
      std::string topicD7 = std::string(systemUrl) + "/" + std::string(IdD7) + "/" + TypeW + "/" + nameRefD7;
      client.subscribe(topicD6.c_str());
      client.subscribe(topicD7.c_str());
    }
    else
    {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
  pinMode(G1, INPUT);
  pinMode(G2, INPUT);
  //   pinMode(G5, INPUT);
  pinMode(G6, OUTPUT);
  pinMode(G7, OUTPUT);

  digitalWrite(G6, LOW);
  digitalWrite(G7, LOW);

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
  readDHT11_1(IdD1);
  readDHT11_1(IdD2);

  delay(5000);
}
