
#include "ESP_AWS_DHT11.h"

sensorData readTemperatureHumidity(DHT dht)
{
    sensorData _sensorData;
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (isnan(h) || isnan(t))
    {
        Serial.println("Failed to read from DHT sensor!");
        _sensorData.temperature = -1;
        _sensorData.humidity = -1;
    }

    _sensorData.temperature = t;
    _sensorData.humidity = h;

    Serial.print("Data from Sensor ");
    Serial.print(_sensorData.temperature);
    Serial.print(", ");
    Serial.println(_sensorData.humidity);
    Serial.println("-------------------");

    return _sensorData;
}

sensorData readFromRTCMemory()
{
    sensorData _sensorData;
    system_rtc_mem_read(RTCMEMORYSTART, &_sensorData, sizeof(_sensorData));

    Serial.print("Data from RTC ");
    Serial.print(_sensorData.temperature);
    Serial.print(", ");
    Serial.println(_sensorData.humidity);
    Serial.println("-------------------");
    yield();
    return _sensorData;
}

void writeToRTCMemory(sensorData rtcMem)
{
    system_rtc_mem_write(RTCMEMORYSTART, &rtcMem, sizeof(rtcMem));

    Serial.print("Saved to RTC ");
    Serial.print(rtcMem.temperature);
    Serial.print(", ");
    Serial.println(rtcMem.humidity);
    Serial.println("-------------------");
    yield();
}

bool isUpdateRequired(sensorData currentData)
{

    if (currentData.temperature == -1 || currentData.humidity == -1)
        return false;

    sensorData previousData = readFromRTCMemory();
    if (previousData.temperature != currentData.temperature || previousData.humidity != currentData.humidity)
    {

        return true;
    }
    return false;
}

void connectWiFi(String init_str)
{
    if (init_str != emptyString)
    {
        Serial.print(init_str);
    }
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(2000);
    }
    if (init_str != emptyString)
    {
        Serial.println("Wifi Connected !");
        Serial.println(WiFi.localIP());
    }
}

void initializeWifi()
{
    WiFi.hostname(THINGNAME);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    connectWiFi(String("Attempting to connect to SSID: ") + String(ssid));
}

void initializeNTP(time_t now)
{

    time_t nowish = 1510592825;
    Serial.print("Setting time using SNTP");
    configTime(5 * 30 * 60, 0, "pool.ntp.org", "time.nist.gov");
    now = time(nullptr);
    while (now < nowish)
    {

        Serial.print("_");
        now = time(nullptr);
        delay(500);
    }
    Serial.println("NTP Sync Completed");

    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    Serial.print("Current time: ");
    Serial.print(asctime(&timeinfo));
}

void pubSubErr(int8_t MQTTErr)
{
    if (MQTTErr == MQTT_CONNECTION_TIMEOUT)
        Serial.print("Connection tiemout");
    else if (MQTTErr == MQTT_CONNECTION_LOST)
        Serial.print("Connection lost");
    else if (MQTTErr == MQTT_CONNECT_FAILED)
        Serial.print("Connect failed");
    else if (MQTTErr == MQTT_DISCONNECTED)
        Serial.print("Disconnected");
    else if (MQTTErr == MQTT_CONNECTED)
        Serial.print("Connected");
    else if (MQTTErr == MQTT_CONNECT_BAD_PROTOCOL)
        Serial.print("Connect bad protocol");
    else if (MQTTErr == MQTT_CONNECT_BAD_CLIENT_ID)
        Serial.print("Connect bad Client-ID");
    else if (MQTTErr == MQTT_CONNECT_UNAVAILABLE)
        Serial.print("Connect unavailable");
    else if (MQTTErr == MQTT_CONNECT_BAD_CREDENTIALS)
        Serial.print("Connect bad credentials");
    else if (MQTTErr == MQTT_CONNECT_UNAUTHORIZED)
        Serial.print("Connect unauthorized");
}

void connectMqtt(PubSubClient &client)
{
    Serial.println();
    Serial.println("Attempting MQTT connection");
    while (!client.connected())
    {
        if (client.connect(THINGNAME))
        {
            Serial.println("MQTT Connection Success!");
            // Send Data
            // sendData(data, client);
        }
        else
        {
            Serial.println("MQTT Connection Failed, Response -> ");
            pubSubErr(client.state());
            Serial.println(" < try again in 5 seconds");
            delay(5000);
        }
    }
}

void sendData(PubSubClient &client, sensorData data)
{

    const char MQTT_PUB_TOPIC[] = "$aws/things/" THINGNAME "/shadow/update";

    DynamicJsonDocument jsonBuffer(JSON_OBJECT_SIZE(3) + 100);
    JsonObject root = jsonBuffer.to<JsonObject>();
    JsonObject state = root.createNestedObject("state");
    JsonObject state_reported = state.createNestedObject("reported");
    state_reported["temperature"] = data.temperature;
    state_reported["humidity"] = data.humidity;
    Serial.printf("Sending  [%s]: ", MQTT_PUB_TOPIC);
    serializeJson(root, Serial);
    Serial.println();
    char shadow[measureJson(root) + 1];
    serializeJson(root, shadow, sizeof(shadow));
    if (!client.publish(MQTT_PUB_TOPIC, shadow, false))
        pubSubErr(client.state());
}