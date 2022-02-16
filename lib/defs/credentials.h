/**
 * @file credentials.h
 * @author SenMorgan https://github.com/SenMorgan
 * @date 2021-11-16
 *
 * @copyright Copyright (c) 2021 Sen Morgan
 *
 * @details All definitions are in /lib/defs/def.h
 *
 */

#ifndef _CREDENTIALS_H_
#define _CREDENTIALS_H_

// Replace with your actual WiFi settings
#define WIFI_SSID   "Your_cool_SSID_name"
#define WIFI_PASSWD "Your_cool_WiFi_password"

// Replace with your actual MQTT settings
#define MQTT_SERVER      "Your_cool_MQTT_server"
// 1883 - default MQTT port
#define MQTT_SERVER_PORT 1883
#define HOSTNAME         "Smart_3_Lights"
#define MQTT_LOGIN       "Your_cool_MQTT_login"
#define MQTT_PASSWORD    "Your_cool_MQTT_password"

// OTA settings
#define OTA_HOSTNAME HOSTNAME
// You can leave it empty if you don't want to secure OTA
#define OTA_PASSWORD ""

#endif // _CREDENTIALS_H_