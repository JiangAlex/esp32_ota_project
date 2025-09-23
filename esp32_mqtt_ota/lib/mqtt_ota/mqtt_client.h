#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <stdint.h>

// public state enum
typedef enum {
  MS_WIFI_CONNECT = 0,
  MS_NET_CHECK,
  MS_MQTT_SUB,
  MS_WAIT_OTA,
  MS_PARSE_JSON,
  MS_OTA_UPDATE,
  MS_REBOOT,
  MS_RETRY,
  MS_RUNNING
} MQTT_STATE;

// initialize module and provide configuration (WiFi / MQTT / OTA)
void mqtt_module_setup(
  const char* wifi_ssid,
  const char* wifi_pass,
  const char* mqtt_server,
  uint16_t mqtt_port,
  const char* mqtt_sub_topic,
  const char* mqtt_pub_topic,
  const char* client_id,
  const char* ota_base_url,
  const char* current_version
);

// state-loop iteration (call frequently). Module handles state transitions until RUNNING.
void mqtt_state_loop_iteration();

// getters used by main when in RUNNING
MQTT_STATE mqtt_get_state();
const char* mqtt_get_pub_topic();
const char* mqtt_get_current_version();

// MQTT helper functions to use in RUNNING state
bool mqtt_connected();
bool mqtt_connect_and_subscribe();
void mqtt_loop();
bool mqtt_publish(const char* topic, const char* payload, size_t len = 0);

#endif // MQTT_CLIENT_H