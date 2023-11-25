
#include "Function.h"

DHT dht1(GATE1, DHT11);
DHT dht2(GATE2, DHT11);


void setup()
{
  Serial.begin(115200);

  // Thiết lập kết nối Wi-Fi
  ConnectWIFI();

  // Thiết lập kết nối MQTT
  ConnectMQTT();
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
  MQTTLoop();
  // gửi tín hiệu
  // readControl(client, GATE6, GATE7);
  readDHT11(dht1, GATE1, dht2, GATE2);

  delay(5000);
}
