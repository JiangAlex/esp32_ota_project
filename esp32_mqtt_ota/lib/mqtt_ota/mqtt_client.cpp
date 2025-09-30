#include "mqtt_client.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

static WiFiClient s_wifiClient;
static PubSubClient* s_mqttClient = nullptr;

// config / state
//static const char* s_wifi_ssid = nullptr;
//static const char* s_wifi_pass = nullptr;
static const char* s_wifi_ssid = "ACC_TPE_WIFI_8F";
static const char* s_wifi_pass = "87738500";
static const char* s_mqtt_server = "192.168.131.55";
static uint16_t s_mqtt_port = 1883;
static const char* s_mqtt_client_id = nullptr;
static const char* s_mqtt_sub_topic = nullptr;
static const char* s_mqtt_pub_topic = nullptr;
static const char* s_ota_base_url = nullptr;
static const char* s_current_version = nullptr;

static String s_lastPayload;

// internal state
static MQTT_STATE s_state = MS_WIFI_CONNECT;
static int s_retryCount = 0;
static unsigned long s_retryStart = 0;
static unsigned long s_lastPublish = 0;
static String s_pendingUrl = String();

// helper to initialize / reset retry counters
static void start_retry_if_needed(bool force = false) {
  if (force || s_retryStart == 0) {
    s_retryStart = millis();
    s_retryCount = 1;
  }
}
static void reset_retry() {
  s_retryStart = 0;
  s_retryCount = 0;
}

static void mqtt_internal_callback(char* topic, byte* payload, unsigned int length) {
  s_lastPayload = "";
  for (unsigned int i = 0; i < length; ++i) s_lastPayload += (char)payload[i];
  // when payload received, state-machine will pick it up on next iteration
}

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
) {
  s_wifi_ssid = wifi_ssid;
  s_wifi_pass = wifi_pass;
  s_mqtt_server = mqtt_server;
  s_mqtt_port = mqtt_port;
  s_mqtt_client_id = client_id;
  s_mqtt_sub_topic = mqtt_sub_topic;
  s_mqtt_pub_topic = mqtt_pub_topic;
  s_ota_base_url = ota_base_url;
  s_current_version = current_version;

  // start WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(s_wifi_ssid, s_wifi_pass);

  // setup mqtt client
  if (s_mqttClient) { delete s_mqttClient; s_mqttClient = nullptr; }
  s_mqttClient = new PubSubClient(s_wifiClient);
  s_mqttClient->setServer(s_mqtt_server, s_mqtt_port);
  s_mqttClient->setCallback(mqtt_internal_callback);
}

bool mqtt_connected() {
  return s_mqttClient ? s_mqttClient->connected() : false;
}

bool mqtt_connect_and_subscribe() {
  if (!s_mqttClient) return false;
  s_mqttClient->setServer(s_mqtt_server, s_mqtt_port);
  const char* cid = s_mqtt_client_id ? s_mqtt_client_id : "esp32_client";
  if (!s_mqttClient->connect(cid)) return false;
  if (s_mqtt_sub_topic && strlen(s_mqtt_sub_topic)) {
    return s_mqttClient->subscribe(s_mqtt_sub_topic);
  }
  return true;
}

void mqtt_loop() {
    Serial.printf("mqtt_loop\n");
    if (s_mqttClient) s_mqttClient->loop();
}

bool mqtt_publish(const char* topic, const char* payload, size_t len) {
  if (!s_mqttClient || !mqtt_connected()) return false;
  if (len == 0) return s_mqttClient->publish(topic, payload);
  return s_mqttClient->publish(topic, (const uint8_t*)payload, (unsigned int)len);
}

MQTT_STATE mqtt_get_state() {
  return s_state;
}
const char* mqtt_get_pub_topic() {
  return s_mqtt_pub_topic;
}
const char* mqtt_get_current_version() {
  return s_current_version;
}

// helper: lightweight internet check
static bool checkInternet() {
  HTTPClient http;
  http.begin("http://clients3.google.com/generate_204");
  http.setTimeout(3000);
  int code = http.GET();
  http.end();
  return (code == 204 || code == 200);
}

static bool isPayloadNewer(const String &payload) {
  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, payload)) return false;
  const char* ver = doc["version"];
  if (!ver) return false;
  return String(ver) != String(s_current_version) && String(ver) > String(s_current_version);
}

static String extractUrl(const String &payload) {
  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, payload)) return String();
  const char* url = doc["url"];
  if (!url) return String();
  return String(url);
}

static void performOTA(const String &url) {
  String otaUrl = url.length() ? url : String(s_ota_base_url ? s_ota_base_url : "");
  if (otaUrl.length() == 0) {
    Serial.println("No OTA URL");
    return;
  }
  t_httpUpdate_return ret = httpUpdate.update(s_wifiClient, otaUrl);
  if (ret == HTTP_UPDATE_OK) {
    Serial.println("OTA update ok -> rebooting");
    delay(500);
    ESP.restart();
  } else {
    Serial.println("OTA failed");
  }
}

// one iteration of state-machine; call frequently from main loop
void mqtt_state_loop_iteration() {
  const int MAX_RETRY = 5;
  const unsigned long RETRY_DELAY_MS = 5000;
  const unsigned long PUBLISH_INTERVAL_MS = 60UL * 1000UL;

  switch (s_state) {
    case MS_WIFI_CONNECT:
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi connected");
        reset_retry();
        s_state = MS_NET_CHECK;
      } else {
        delay(200);
        start_retry_if_needed();
        s_state = MS_RETRY;
      }
      break;

    case MS_NET_CHECK:
      if (checkInternet()) {
        Serial.println("Internet OK");
        reset_retry();
        s_state = MS_MQTT_SUB;
      } else {
        Serial.println("Internet fail");
        start_retry_if_needed();
        s_state = MS_RETRY;
      }
      break;

    case MS_MQTT_SUB:
      if (mqtt_connect_and_subscribe()) {
        Serial.println("MQTT connected & subscribed");
        reset_retry();
        s_lastPublish = millis();
        s_state = MS_WAIT_OTA;
      } else {
        Serial.println("MQTT connect fail");
        start_retry_if_needed();
        s_state = MS_RETRY;
      }
      break;

    case MS_WAIT_OTA:
      if (!mqtt_connected()) {
        Serial.println("MQTT disconnected while waiting");
        s_state = MS_RETRY;
        s_retryStart = millis();
        s_retryCount++;
        break;
      }
      mqtt_loop();

      if (millis() - s_lastPublish >= PUBLISH_INTERVAL_MS) {
        s_lastPublish = millis();
        String payload = String("{\"id\":\"") + WiFi.macAddress() + String("\",\"version\":\"") + String(s_current_version) + String("\"}");
        mqtt_publish(s_mqtt_pub_topic, payload.c_str(), payload.length());
        Serial.println("Published update request");
      }

      if (s_lastPayload.length()) {
        s_state = MS_PARSE_JSON;
      }
      break;

    case MS_PARSE_JSON:
      if (s_lastPayload.length() == 0) {
        Serial.println("Payload fail");
        s_state = MS_WAIT_OTA;
        break;
      }
      if (isPayloadNewer(s_lastPayload)) {
        s_pendingUrl = extractUrl(s_lastPayload);
        Serial.printf("New OTA payload found: %s\n", s_pendingUrl.c_str());
        s_lastPayload = "";
        s_state = MS_OTA_UPDATE;
      } else {
        Serial.println("No new OTA payload");
        s_lastPayload = "";
        s_state = MS_WAIT_OTA;
      }
      break;

    case MS_OTA_UPDATE:
      if (s_pendingUrl.length() == 0) {
        Serial.println("No OTA URL available");
      } else {
        performOTA(s_pendingUrl);
      }
      s_pendingUrl = "";
      // after OTA attempt, go to retry logic to re-establish if needed
      // force start a fresh retry flow
      start_retry_if_needed(true);
      s_state = MS_RETRY;
      break;

    case MS_RETRY:
      if (s_retryCount >= MAX_RETRY) {
        Serial.println("Max retries reached, entering RUNNING state.");
        s_state = MS_RUNNING;
      } else {
        unsigned long delayTime = RETRY_DELAY_MS * (1UL << (max(0, s_retryCount - 1)));
        if (millis() - s_retryStart >= delayTime) {
          s_retryCount++;
          Serial.printf("Retry attempt #%d\n", s_retryCount);
          s_state = MS_WIFI_CONNECT;
        }
      }
      break;

    case MS_REBOOT:
      Serial.println("Rebooting...");
      delay(1000);
      ESP.restart();
      break;

    case MS_RUNNING:
      // module yields control to main loop for normal running behavior
      // do nothing here; main will call mqtt_loop() and publish
      break;
  }
}