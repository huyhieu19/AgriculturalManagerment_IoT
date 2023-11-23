// For ESP8266 CP210x
#include <StreamString.h>
#include <inttypes.h>
#include <ESP8266WiFi.h>
#include "PubSubClient.h"
#include <Adafruit_Sensor.h>
#include "DHT.h"
#include "Wire.h"
#include <Arduino.h>
#include <string>

typedef uint8_t byte;

#ifndef FUNCTION_H
#define FUNCTION_H

// khơi tạo client và wifi

WiFiClient espClient;
PubSubClient client(espClient);

// khởi tạo dht 11
// extern const int GpioD5 = D5;

// Thông tin Wifi
#define ssid "FPT Telecom.2.4G"
#define password "22121999"

// Thông tin kết nối MQTT broker
// extern const char *mqttServer = "broker.hivemq.com";
// extern const int mqttPort = 1883;
// extern const char *mqttUser = "your_MQTT_username";
// extern const char *mqttPassword = "your_MQTT_password";

// Thông tin Id hệ thống
#define systemUrl "3c531531-d5f5-4fe3-9954-5afd76ff2151"

// Thông tin Id các chân
#define IdD1 "ff824d3a-2548-4f16-b111-d102d3a3cdb4"
#define nameRefD1_1 "ND"
#define nameRefD1_2 "DA"
#define G1 D1

#define IdD2 "xxx"
#define nameRefD2_1 "ND"
#define nameRefD2_2 "DA"
#define G2 D2

#define IdD5 = "xxx"
#define nameRefD5_1 = "AD"
#define G5 = D5

#define IdD6 "xxx"
#define nameRefD6 "O_F"
#define G6 D6

#define IdD7 "xxx"
#define nameRefD7 "O_F"
#define G7 D7

#define TypeR "R"
#define TypeW "W"

extern DHT dht(G1, DHT11);

void reconnect();
void readControl();
void readDHT11_1(char *Id);
void splitTopic(String topic, String *topicArray, int arraySize);
void callback(char *topic, byte *payload, unsigned int length);

#endif