#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <Update.h>
#include <ArduinoJson.h>

// ====== WiFi/MQTT Config ======
const char* WIFI_SSID     = "AlexHome2G";
const char* WIFI_PASS     = "26743819";
const char* MQTT_BROKER   = "192.168.2.223";
const int   MQTT_PORT     = 1883;
const char* DEVICE_ID     = "esp32-01";
String MQTT_TOPIC_ALL     = "esp32/update";
String MQTT_TOPIC_DEVICE  = "esp32/update/" + String(DEVICE_ID);

// ====== 狀態機 ======
enum State {
  WIFI_CONNECT,
  NET_CHECK,
  MQTT_SUB,
  WAIT_OTA,
  PARSE_JSON,
  OTA_UPDATE,
  REBOOT,
  RETRY
};

State currentState = WIFI_CONNECT;
WiFiClient espClient;
PubSubClient client(espClient);

// ====== 0922 新增: 重試計數 ======
int retryCount = 0;
const int MAX_RETRY = 5;

// ====== 函數宣告 ======
bool checkInternet();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void runOtaUpdate(String fwUrl, String newVersion);

// ====== Global ======
unsigned long lastUpdateRequest = 0;
const unsigned long updateInterval = 60000; // 60 秒

// ====== setup ======
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32 OTA FSM Start");
  client.setServer(MQTT_BROKER, MQTT_PORT);
  client.setCallback(mqttCallback);
}

// ====== loop ======
void loop() {
  switch (currentState) {

    case WIFI_CONNECT:
      Serial.println("Connecting WiFi...");
      WiFi.begin(WIFI_SSID, WIFI_PASS);
      if (WiFi.waitForConnectResult() == WL_CONNECTED) {
        retryCount = 0; // 成功，重置重試計數 0922
        currentState = NET_CHECK;
      } else {
        currentState = RETRY;
      }
      break;

    case NET_CHECK:
      Serial.println("Checking Internet...");
      if (checkInternet()) {
        retryCount = 0; // 成功，重置重試計數 0922
        currentState = MQTT_SUB;
      } else {
        currentState = RETRY;
      }
      break;

    case MQTT_SUB:
      Serial.println("Connecting MQTT...");
      if (client.connect(DEVICE_ID)) {
        retryCount = 0; // 成功，重置重試計數 0922
        client.subscribe(MQTT_TOPIC_ALL.c_str());
        client.subscribe(MQTT_TOPIC_DEVICE.c_str());
        Serial.println("Subscribed to MQTT topics");
        currentState = WAIT_OTA;
        String triggerTopic = "esp32/update/" + String(DEVICE_ID);
        client.publish(triggerTopic.c_str(), "update");
        Serial.println("📤 Sent update request → " + triggerTopic);
      } else {
        currentState = RETRY;
      }
      break;
// ====== 在 loop() 中 WAIT_OTA 狀態處理 ======
    case WAIT_OTA:
      client.loop();  // keep MQTT alive

    // 定時主動要求更新
    if (millis() - lastUpdateRequest > updateInterval) {
      lastUpdateRequest = millis();
      String triggerTopic = "esp32/update/" + String(DEVICE_ID);
      client.publish(triggerTopic.c_str(), "update");
      Serial.println("📤 Sent update request → " + triggerTopic);
      
    }
      break;

    case PARSE_JSON:
      // 由 mqttCallback 切換狀態
      break;

    case OTA_UPDATE:
      // runOtaUpdate() 會負責，完成後切 REBOOT
      break;

    case REBOOT:
      ESP.restart();
      break;

    case RETRY:  //0922 修改
      retryCount++;
      Serial.printf("Retry #%d in 5s...\n", retryCount);
      if (retryCount >= MAX_RETRY) {
        // 超過最大重試次數，改為進入 RUNNING（WAIT_OTA）繼續執行
        Serial.println("Max retries reached. Entering RUNNING (WAIT_OTA) state.");
        retryCount = 0;
        currentState = WAIT_OTA;
      } else {
        delay(5000);
        currentState = WIFI_CONNECT;
      }
      break;
  }
}

// ====== Function: 檢查外部網路 ======
bool checkInternet() {
  HTTPClient http;
  http.begin("http://www.google.com.tw/");  // 測試用
  int httpCode = http.GET();
  http.end();
  return (httpCode > 0 && httpCode < 400);
}

// ====== Function: MQTT callback ======
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  Serial.printf("MQTT [%s] %s\n", topic, msg.c_str());

  // 假設 payload: {"version":"1.1.0","url":"http://example.com/fw.bin"}
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, msg);
  if (error) {
    Serial.println("JSON parse failed!");
    return;
  }

  String newVer = doc["version"].as<String>();
  String fwUrl  = doc["url"].as<String>();

  // 假設當前版本用 "1.0.0"
  if (newVer != "1.0.0") {
    runOtaUpdate(fwUrl, newVer);
  } else {
    Serial.println("Already latest version");
  }
}

// ====== Function: OTA 更新 ======
void runOtaUpdate(String fwUrl, String newVersion) {
  Serial.printf("Start OTA from %s\n", fwUrl.c_str());
  WiFiClientSecure clientSecure;
  clientSecure.setInsecure();

  HTTPClient http;
  http.begin(clientSecure, fwUrl);

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    int len = http.getSize();
    WiFiClient* stream = http.getStreamPtr();

    if (Update.begin(len)) {
      size_t written = Update.writeStream(*stream);
      if (written == len) {
        Serial.println("OTA Success!");
        if (Update.end()) {
          currentState = REBOOT;
        }
      }
    }
  } else {
    Serial.println("OTA Failed, HTTP Error");
    currentState = RETRY;
  }
  http.end();
}
