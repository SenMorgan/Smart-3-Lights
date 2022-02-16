/**
 * @file def.h
 * @author SenMorgan https://github.com/SenMorgan
 * @date 2021-11-16
 *
 * @copyright Copyright (c) 2021 Sen Morgan
 *
 */

#ifndef _DEF_H_
#define _DEF_H_

// MQTT definitions
#define DEFAULT_TOPIC             "/smart-3-lights/"
#define MQTT_WILL_TOPIC           DEFAULT_TOPIC "availability"
#define MQTT_QOS                  1
#define MQTT_RETAIN               1
#define MQTT_WILL_MESSAGE         DEFAULT_TOPIC "offline"
#define MQTT_SUBSCRIBE_TOPIC      DEFAULT_TOPIC "#"
#define MQTT_PUBLISH_TOPIC        DEFAULT_TOPIC "state"
#define MQTT_AVAILABILITY_TOPIC   DEFAULT_TOPIC "availability"
#define MQTT_AVAILABILITY_MESSAGE "online"
#define MQTT_UPTIME_TOPIC         DEFAULT_TOPIC "uptime"
#define MQTT_CMD_TOPIC            DEFAULT_TOPIC "set"
#define MQTT_CMD_ON               "1"
#define MQTT_CMD_OFF              "0"

// Reconnecting to MQTT server delay
#define RECONNECT_DELAY     5000
// Delay between reporting states to MQTT server in idle mode (ms)
#define PUBLISH_STEP        30000
// Run lights effect delay
#define RUN_LIGHTS_STEP     120000L
// Run lights effect interval between switching
#define RUN_LIGHTS_INTERVAL 100
// Run lights effect repeat times
#define RUN_LIGHTS_CYCLES   2

// IO pins
#define STATUS_LED D4
#define LIGHT_1    D5
#define LIGHT_2    D6
#define LIGHT_3    D7

#endif // _DEF_H_