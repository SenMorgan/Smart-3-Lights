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
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>

WiFiManager WiFiMan;

WiFiClient espClient;
PubSubClient mqttClient(espClient);
