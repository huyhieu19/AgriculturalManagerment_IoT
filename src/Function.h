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

const u_int8_t GATE1 = D1;
const u_int8_t GATE2 = D2;
const u_int8_t GATE5 = D5;
const u_int8_t GATE6 = D6;
const u_int8_t GATE7 = D7;

// Định nghĩa kích thước của từng khu vực Flash
#define AREA_SIZE 512

// Định nghĩa địa chỉ bắt đầu của từng khu vực Flash
#define AREA1_ADDRESS 0
#define AREA2_ADDRESS (AREA1_ADDRESS + AREA_SIZE)
#define AREA3_ADDRESS (AREA2_ADDRESS + AREA_SIZE)
#define AREA4_ADDRESS (AREA3_ADDRESS + AREA_SIZE)

// All connection
void ConnectWIFI();
void ConnectMQTT(PubSubClient client);
void reconnect(PubSubClient client);

// Extention
void splitTopic(String topic, String *topicArray, int arraySize);
void callback(char *topic, byte *payload, unsigned int length);

void readControl(PubSubClient client, uint8_t GateD6, uint8_t GateD7);
void readDHT11(PubSubClient client, DHT dht1, uint8_t GateDHT1, DHT dht2, uint8_t GateDHT2);
void StartDHT(DHT dht1, DHT dht2);

// Write data to flash memory
void WriteEEPROM(char *topic, byte *payload, unsigned int length);
void ReadEEPROM(char *topic);

#endif