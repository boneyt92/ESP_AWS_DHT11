#ifndef ESP_AWS_DHT11_H
#define ESP_AWS_DHT11_H

#include <DHT.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "secrets.h"

extern "C"
{
#include "user_interface.h"
}

#define emptyString String()
#define RTCMEMORYSTART 65

typedef struct
{
    float temperature;
    float humidity;
} sensorData;

const int MQTT_PORT = 8883;

sensorData readTemperatureHumidity(DHT);
sensorData readFromRTCMemory();
void writeToRTCMemory(sensorData);
bool isUpdateRequired(sensorData);
void initializeWifi();
void connectWiFi(String);
void initializeNTP(time_t);
void connectMqtt(PubSubClient &client);
void pubSubErr(int8_t);
void sendData(PubSubClient &client, sensorData);

#endif