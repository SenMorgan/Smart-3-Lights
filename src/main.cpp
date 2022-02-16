/**
 * @file main.cpp
 * @author SenMorgan https://github.com/SenMorgan
 * @brief Smart 3 Lights
 * @version 0.1
 * @date 2021-11-16
 *
 * @copyright Copyright (c) 2021 Sen Morgan
 *
 * @details All definitions are in /lib/defs/def.h
 *          All credentials are in /lib/defs/credentials.h
 *
 */

#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "credentials.h"
#include "def.h"

WiFiClient   espClient;
PubSubClient mqttClient(espClient);

static uint32_t publishTimeStamp;

// EEPROM memory structure
struct
{
    uint32_t resetCounter = 0;
    uint8_t  mainState    = 0;
} params;

/**
 * @brief Initializing the Lights object
 *
 * @param newState
 */
void init_lights(uint8_t newState)
{
    params.mainState = newState;
    if (newState)
    {
        // Relay have negative active input
        digitalWrite(LIGHT_1, 0);
        delay(500);
        digitalWrite(LIGHT_2, 0);
        delay(500);
        digitalWrite(LIGHT_3, 0);
    }
    else
    {
        digitalWrite(LIGHT_1, 1);
        digitalWrite(LIGHT_2, 1);
        digitalWrite(LIGHT_3, 1);
        delay(1000);
    }
}

/**
 * @brief Set the Lights object
 *
 * @param newState New state of lights
 */
void set_lights(uint8_t newState)
{
    params.mainState = newState;
    EEPROM.put(0, params);
    EEPROM.commit();
    if (newState)
    {
        // Relay have negative active input
        digitalWrite(LIGHT_1, 0);
        delay(1000);
        digitalWrite(LIGHT_2, 0);
        delay(1000);
        digitalWrite(LIGHT_3, 0);
    }
    else
    {
        digitalWrite(LIGHT_1, 1);
        delay(1000);
        digitalWrite(LIGHT_2, 1);
        delay(1000);
        digitalWrite(LIGHT_3, 1);
    }
    // Push data to server immediately
    publishTimeStamp = 0;
}

/**
 * @brief Running lights effect
 *
 * @param state Lights actual state
 * @param ticks Actul ticks
 */
void run_lights(uint8_t state, uint32_t ticks)
{
    static uint32_t timeStamp = 0;

    if (state)
    {
        if (!timeStamp)
            timeStamp = ticks;
        else if (ticks > timeStamp + RUN_LIGHTS_STEP)
        {
            timeStamp = ticks;
            for (int i = 0; i < RUN_LIGHTS_CYCLES; i++)
            {
                digitalWrite(LIGHT_1, 1);
                delay(RUN_LIGHTS_INTERVAL);
                digitalWrite(LIGHT_1, 0);
                delay(RUN_LIGHTS_INTERVAL);
                digitalWrite(LIGHT_2, 1);
                delay(RUN_LIGHTS_INTERVAL);
                digitalWrite(LIGHT_2, 0);
                delay(RUN_LIGHTS_INTERVAL);
                digitalWrite(LIGHT_3, 1);
                delay(RUN_LIGHTS_INTERVAL);
                digitalWrite(LIGHT_3, 0);
            }
        }
    }
    else if (timeStamp > 0)
        timeStamp = 0;
}

/**
 * @brief Connecting to MQTT server
 *
 * @return true if successfully reconnected to MQTT server, otherwise false
 */
uint8_t reconnect(void)
{
    if (mqttClient.connect(HOSTNAME, MQTT_LOGIN, MQTT_PASSWORD,
                           MQTT_WILL_TOPIC, MQTT_QOS, MQTT_RETAIN, MQTT_WILL_MESSAGE))
    {
        mqttClient.subscribe(MQTT_SUBSCRIBE_TOPIC);
        Serial.println("Successfully connected to " MQTT_SERVER);
        return 1;
    }
    Serial.println("Can't connect to MQTT server...");
    return 0;
}

/**
 * @brief MQTT Callback
 */
void callback(String topic, byte *payload, uint16_t length)
{
    String msgString = "";
    for (uint16_t i = 0; i < length; i++)
        msgString += (char)payload[i];

    // Commands topic
    if (topic == MQTT_CMD_TOPIC)
    {
        if (msgString == MQTT_CMD_ON)
        {
            mqttClient.publish(MQTT_PUBLISH_TOPIC, MQTT_CMD_ON, true);
            set_lights(1);
            Serial.println("Received command ON");
        }
        else if (msgString == MQTT_CMD_OFF)
        {
            mqttClient.publish(MQTT_PUBLISH_TOPIC, MQTT_CMD_OFF, true);
            set_lights(0);
            Serial.println("Received command OFF");
        }
    }
}

/**
 * @brief Publish data to server
 */
void publish_data(void)
{
    static char buff[20];

    // Publish current lights state to server
    mqttClient.publish(MQTT_PUBLISH_TOPIC, (params.mainState ? MQTT_CMD_ON : MQTT_CMD_OFF), true);

    mqttClient.publish(MQTT_AVAILABILITY_TOPIC, MQTT_AVAILABILITY_MESSAGE);
    sprintf(buff, "%ld sec", millis() / 1000);
    mqttClient.publish(MQTT_UPTIME_TOPIC, buff);

    Serial.println("Data was sended, time from start: " + String(buff));
}

void setup(void)
{
    pinMode(STATUS_LED, OUTPUT);
    pinMode(LIGHT_1, OUTPUT);
    pinMode(LIGHT_2, OUTPUT);
    pinMode(LIGHT_3, OUTPUT);

    digitalWrite(STATUS_LED, 1);
    digitalWrite(LIGHT_1, 1);
    digitalWrite(LIGHT_2, 1);
    digitalWrite(LIGHT_3, 1);

    EEPROM.begin(16);
    // Read data from EEPROM
    EEPROM.get(0, params);

    // Switch on / leave off lights depending on inverted saved value
    init_lights(!params.mainState);

    // Increment reset counter and save it to EEPROM
    params.resetCounter++;
    EEPROM.put(0, params);
    EEPROM.commit();

    // Serial port initializing
    Serial.begin(115200);
    Serial.println("\n Starting");

    // MQTT initializing
    mqttClient.setServer(MQTT_SERVER, MQTT_SERVER_PORT);
    mqttClient.setCallback(callback);
    Serial.println("Connecting to MQTT server...");
    if (reconnect())
        publish_data();

    // Wi-Fi initializing
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    WiFi.hostname(HOSTNAME);

    // Arduino OTA initializing
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.begin();
    ArduinoOTA.onProgress([](uint16_t progress, uint16_t total)
                          { digitalWrite(STATUS_LED, !digitalRead(STATUS_LED)); });
    ArduinoOTA.onEnd([]()
                     { digitalWrite(STATUS_LED, 1); });

    // Make sure that status led is off
    digitalWrite(STATUS_LED, 1);
}

void loop(void)
{
    ArduinoOTA.handle();

    uint32_t timeNow           = millis();
    uint8_t  connectedToServer = mqttClient.loop();

    // If it is time to publish data
    if (timeNow - publishTimeStamp > PUBLISH_STEP)
    {
        publishTimeStamp = timeNow;
        // Send data if we are connected to MQTT server
        if (connectedToServer)
        {
            digitalWrite(STATUS_LED, 0); // blink buildin LED
            publish_data();              // publish data
            digitalWrite(STATUS_LED, 1); // switch off buildin LED
        }
        // Or we can try to reconnect
        else
        {
            reconnect();
        }
    }
    // run_lights(params.mainState, timeNow);
    yield();
}