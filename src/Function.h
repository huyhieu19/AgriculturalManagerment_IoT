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

// All connection
void ConnectWIFI(char *SSID, char *WIFIPW);
void ConnectMQTT(PubSubClient client, char *MQTTSV, uint16_t MQTTPORT, char *topics[]);
void reconnect(PubSubClient client, char *MQTTUSER, char *MQTTPW, char *topics[]);

// Extention
void splitTopic(String topic, String *topicArray, int arraySize);
void callback(char *topic, byte *payload, unsigned int length);

void readControl(PubSubClient client, char *topicD6, uint8_t GateD6, char *topicD7, uint8_t GateD7);
void readDHT11(PubSubClient client, DHT dht1, uint8_t GateDHT1, char *topicD1s[], DHT dht2, uint8_t GateDHT2, char *topicD2s[]);
void StartDHT(DHT dht1, DHT dht2);

// Write data to flash memory
// void WriteEEPROM(char *topic, byte *payload, unsigned int length);
// void ReadEEPROM(char *topic);

#endif