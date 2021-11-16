/**************************************************************
 *  def.h
 *
 *  Created on: 16 November 2021
 *  Author: SenMorgan https://github.com/SenMorgan
 *  
 ***************************************************************/

#ifndef _DEF_H
#define _DEF_H

#define DEFAULT_HOSTNAME          "Smart_3_Lights"
// How many times should reset button be pressed to call setting erase
#define RESET_BUTTON_CYCLES       3
// Maximum delay between pressing reset button to call setting erase
#define RESET_BUTTON_DELAY        500
// MQTT definitions
#define DEFAULT_TOPIC             "/smart-3-lights/"
#define MQTT_WILL_TOPIC           "availability"
#define MQTT_QOS                  1
#define MQTT_RETAIN               1
#define MQTT_WILL_MESSAGE         "offline"
#define MQTT_SUBSCRIBE_TOPIC      "#"
#define MQTT_PUBLISH_TOPIC        "state"
#define MQTT_AVAILABILITY_TOPIC   "availability"
#define MQTT_AVAILABILITY_MESSAGE "online"
#define MQTT_UPTIME_TOPIC         "uptime"
#define MQTT_CMD_TOPIC            "set"
// Reconnecting to MQTT server delay
#define RECONNECT_DELAY           30000
// Delay between reporting states to MQTT server in idle mode (ms)
#define PUBLISH_STEP              30000
// IO pins
#define LIGHT_1                   D1
#define LIGHT_2                   D2
#define LIGHT_3                   D3
#define STATUS_LED                D4

#endif /* _DEF_H */