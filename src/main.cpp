/**************************************************************
 *  main.cpp
 *
 *  Created in PlatformIO: 16 November 2021
 *  Author: SenMorgan https://github.com/SenMorgan
 * 
 *  All definitions are in /lib/defs/def.h
 *  
 ***************************************************************/

#include "def.h"
#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

WiFiManager WiFiMan;

WiFiClient   espClient;
PubSubClient mqttClient(espClient);

static uint32_t publishTimeStamp;
static uint8_t  saveToEEPROMflag;

class IntParameter : public WiFiManagerParameter
{
public:
    IntParameter(const char *id, const char *placeholder, long value, const uint8_t length = 10)
        : WiFiManagerParameter("")
    {
        init(id, placeholder, String(value).c_str(), length, "", WFM_LABEL_BEFORE);
    }

    long getValue()
    {
        return String(WiFiManagerParameter::getValue()).toInt();
    }
};

// EEPROM memory structure
struct
{
    char     hostname[32]     = "";
    char     otaPass[32]      = "";
    char     mqttServer[32]   = "";
    char     mqttPort[6]      = "";
    char     mqttTopic[32]    = "";
    char     mqttLogin[32]    = "";
    char     mqttPass[32]     = "";
    uint32_t resetCounter     = 0;
    uint32_t resetCounterComp = 0;
    uint8_t  mainState        = 0;
} params;

// Headers to be appended with topics
char willTopic[128]          = "";
char subscribeTopic[128]     = "";
char cmdTopic[128]           = "";
char publishTopic[128]       = "";
char availabililtyTopic[128] = "";
char uptimeTopic[128]        = "";

/**
 * @brief Set the Lights object
 * 
 * @param newState 
 */
void setLights(uint8_t newState)
{
    params.mainState = newState;
    EEPROM.put(0, params);
    EEPROM.commit();
    // Relay have negative active input
    digitalWrite(LIGHT_1, !newState);
    delay(500);
    digitalWrite(LIGHT_2, !newState);
    delay(500);
    digitalWrite(LIGHT_3, !newState);
    // For updating data immediately
    publishTimeStamp = 0;
}

/**
 * @brief Connecting to MQTT server
 * 
 * @return true if successfully reconnected to MQTT server
 * @return false if not
 */
uint8_t reconnect(void)
{
    if (mqttClient.connect(params.hostname, params.mqttLogin, params.mqttPass,
                           willTopic, MQTT_QOS, MQTT_RETAIN, MQTT_WILL_MESSAGE))
    {
        mqttClient.subscribe(subscribeTopic);
        Serial.println("Successfully connected to " +
                       String(params.mqttServer) + ":" + String(params.mqttPort));
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
    if (topic == cmdTopic)
    {
        if (msgString == MQTT_CMD_ON)
        {
            mqttClient.publish(publishTopic, MQTT_CMD_ON, true);
            setLights(1);
            Serial.println("Received command ON");
        }
        else if (msgString == MQTT_CMD_OFF)
        {
            mqttClient.publish(publishTopic, MQTT_CMD_OFF, true);
            setLights(0);
            Serial.println("Received command OFF");
        }
    }
}

/**
 * @brief Publish data to server
 * 
 */
void publish_data(void)
{
    static char buff[20];

    // Publish current lights state to server
    mqttClient.publish(publishTopic, (params.mainState ? MQTT_CMD_ON : MQTT_CMD_OFF), true);

    mqttClient.publish(availabililtyTopic, MQTT_AVAILABILITY_MESSAGE);
    sprintf(buff, "%ld sec", millis() / 1000);
    mqttClient.publish(uptimeTopic, buff);

    Serial.println("Data was sended, time from start: " + String(buff));
}

/**
 * @brief Callback notifying us of the need to save config
 * 
 */
void saveConfigCallback(void)
{
    Serial.println("Saving to EEPROM...");
    saveToEEPROMflag = true;
    WiFiMan.stopConfigPortal();
}

/**
 * @brief Clearing all saved values (WiFi, hostname, MQTT)
 * 
 */
void clear_setting_memory(void)
{
    for (int i = 0; i < 512; i++)
    {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    delay(400);
    WiFiMan.resetSettings();
    delay(100);
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

    EEPROM.begin(512);
    // Read data from EEPROM
    EEPROM.get(0, params);

    // Increment reset counter and save it to EEPROM
    params.resetCounter++;
    EEPROM.put(0, params);
    EEPROM.commit();

    // Switch on / leave off lights depending on inverted saved value
    setLights(!params.mainState);

    // Some delay for the user to be able to call EEPROM erase command
    // by pressing reset button a few times
    // delay(RESET_BUTTON_DELAY);

    Serial.begin(115200);
    Serial.println("\n Starting");

    // If it is first boot or if reset counter is empty (cleaned EEPROM)
    if (params.resetCounter == 1)
    {
        Serial.println("Setting default values");
        params.resetCounterComp = params.resetCounter;
        // Set and save default parameters
        strcpy(params.hostname, DEFAULT_HOSTNAME);
        strcpy(params.mqttTopic, DEFAULT_TOPIC);
        params.mainState = 0;
        EEPROM.put(0, params);
        EEPROM.commit();
        delay(10);
    }
    // If reset buton was pressed more than RESET_BUTTON_CYCLES times quickly
    else if ((params.resetCounter - params.resetCounterComp) >= RESET_BUTTON_CYCLES)
    {
        Serial.println("Erasing WiFi and MQTT settings!");
        clear_setting_memory();
        // blink buildin LED many times
        for (int i = 0; i < 50; i++)
        {
            digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
            delay(50);
        }
        ESP.restart();
    }
    // If reset button wasn't pressed enough quicly - save same value
    params.resetCounterComp = params.resetCounter;
    EEPROM.put(0, params);
    EEPROM.commit();

    // Print saved data
    Serial.println("Reading from EEPROM...");
    Serial.println("Hostname: " + String(params.hostname));
    Serial.println("OTA password: " + String(params.otaPass));
    Serial.println("MQTT server: " + String(params.mqttServer));
    Serial.println("MQTT port: " + String(params.mqttPort));
    Serial.println("MQTT topic: " + String(params.mqttTopic));
    Serial.println("MQTT login: " + String(params.mqttLogin));
    Serial.println("MQTT password: " + String(params.mqttPass));
    Serial.println("Resets count: " + String(params.resetCounter));
    Serial.println("Resets count compare: " + String(params.resetCounterComp));

    // Setup some web configuration parameters
    WiFiManagerParameter custom_device_settings("<p>Device Settings</p>");
    WiFiManagerParameter custom_hostname("hostname", "Device Hostname", params.hostname, 32);
    WiFiManagerParameter custom_ota_pass("otaPass", "OTA Password", params.otaPass, 32);

    WiFiManagerParameter custom_mqtt_settings("<p>MQTT Settings</p>");
    WiFiManagerParameter custom_mqtt_server("mqttServer", "MQTT Server", params.mqttServer, 32);
    WiFiManagerParameter custom_mqtt_port("mqttPort", "MQTT Port", params.mqttPort, 6);
    WiFiManagerParameter custom_mqtt_topic("mqttTopic", "MQTT Topic", params.mqttTopic, 32);
    WiFiManagerParameter custom_mqtt_login("mqttLogin", "MQTT Login", params.mqttLogin, 32);
    WiFiManagerParameter custom_mqtt_pass("mqttPass", "MQTT Password", params.mqttPass, 32);

    // Callbacks
    WiFiMan.setSaveConfigCallback(saveConfigCallback);

    // Add all parameters
    WiFiMan.addParameter(&custom_device_settings);
    WiFiMan.addParameter(&custom_hostname);
    WiFiMan.addParameter(&custom_ota_pass);
    WiFiMan.addParameter(&custom_mqtt_settings);
    WiFiMan.addParameter(&custom_mqtt_server);
    WiFiMan.addParameter(&custom_mqtt_port);
    WiFiMan.addParameter(&custom_mqtt_topic);
    WiFiMan.addParameter(&custom_mqtt_login);
    WiFiMan.addParameter(&custom_mqtt_pass);

    // Invert theme, dark
    WiFiMan.setDarkMode(true);
    // Show scan RSSI as percentage, instead of signal stength graphic
    WiFiMan.setScanDispPerc(true);
    // Set hostname as saved one
    WiFiMan.setHostname(params.hostname);
    // Set timeout until configuration portal gets turned off
    WiFiMan.setConfigPortalTimeout(180);
    // Needed to use saveWifiCallback
    WiFiMan.setBreakAfterConfig(true);
    // Use autoconnect and start web portal
    if (!WiFiMan.autoConnect(params.hostname))
    {
        Serial.println("failed to connect and hit timeout");
    }
    // If new params in config portal was changed - save them to EEPROM
    if (saveToEEPROMflag)
    {
        strcpy(params.hostname, custom_hostname.getValue());
        strcpy(params.otaPass, custom_ota_pass.getValue());
        strcpy(params.mqttServer, custom_mqtt_server.getValue());
        strcpy(params.mqttPort, custom_mqtt_port.getValue());
        strcpy(params.mqttTopic, custom_mqtt_topic.getValue());
        strcpy(params.mqttLogin, custom_mqtt_login.getValue());
        strcpy(params.mqttPass, custom_mqtt_pass.getValue());

        EEPROM.put(0, params);
        EEPROM.commit();
        Serial.println("Saved!");
    }

    // Appending topics
    strcpy(willTopic, params.mqttTopic);
    strcpy(subscribeTopic, params.mqttTopic);
    strcpy(cmdTopic, params.mqttTopic);
    strcpy(publishTopic, params.mqttTopic);
    strcpy(availabililtyTopic, params.mqttTopic);
    strcpy(uptimeTopic, params.mqttTopic);

    strcat(willTopic, MQTT_WILL_TOPIC);
    strcat(subscribeTopic, MQTT_SUBSCRIBE_TOPIC);
    strcat(cmdTopic, MQTT_CMD_TOPIC);
    strcat(publishTopic, MQTT_PUBLISH_TOPIC);
    strcat(availabililtyTopic, MQTT_AVAILABILITY_TOPIC);
    strcat(uptimeTopic, MQTT_UPTIME_TOPIC);

    // MQTT initializing
    mqttClient.setServer(params.mqttServer, atoi(params.mqttPort));
    mqttClient.setCallback(callback);
    Serial.println("Connecting to MQTT server...");
    if (reconnect())
        publish_data();

    // Arduino OTA initializing
    ArduinoOTA.setHostname(params.hostname);
    //ArduinoOTA.setPassword(params.otaPass);
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
    yield();
}