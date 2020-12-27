
#include "config.h"
#include "ESP_AWS_DHT11.h"

DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure net;
PubSubClient client(net);

time_t now;

BearSSL::X509List cert(cacert);
BearSSL::X509List client_crt(client_cert);
BearSSL::PrivateKey key(privkey);

void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(2000);

  while (!Serial)
  {
  }
  Serial.println();
  pinMode(LED, OUTPUT);
  dht.begin();

  sensorData data = readTemperatureHumidity(dht);

  if (isUpdateRequired(data))
  {
    Serial.println("Value Changed. Update now");

    writeToRTCMemory(data);
    initializeWifi();
    initializeNTP(now);

    net.setTrustAnchors(&cert);
    net.setClientRSACert(&client_crt, &key);

    client.setServer(MQTT_HOST, MQTT_PORT);

    connectMqtt(client);

    sendData(client, data);
  }
  else
  {
    Serial.println("Same value. No Update Required");
  }
  Serial.println("DeepSleep for 15s");

  ESP.deepSleep(15e6);
}

void loop()
{
  // put your main code here, to run repeatedly:
}