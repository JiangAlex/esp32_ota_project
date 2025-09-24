/*
  Minimal state-machine implementation driven by docs/state_diagram.mmd
  (state-loop moved into mqtt_client module)

main.cpp 現在只留 setup() 與 loop()，loop() 中包含 RUNNING 狀態的常態處理（維持 mqtt 連線、mqtt_loop()、以及週期 publish）。
mqtt_client.cpp 含完整的狀態機與 WiFi/MQTT/OTA 邏輯（已移除 main 中的輔助函式）。
若需調整 WiFi/MQTT/OTA 的參數，請修改 setup() 內呼叫 mqtt_module_setup(...) 的字串參數。
*/

#include <Arduino.h>
#include "mqtt_client.h"

void setup() {
  Serial.begin(115200);
  delay(100);

  // 初始化 mqtt module (包含 WiFi/MQTT/OTA 等邏輯設定)
  // 把你的參數放在這裡 — 保持 main 簡潔
  mqtt_module_setup(
    "your_ssid",
    "your_password",
    "mqtt.example.com",
    1883,
    "device/ota/cmd",
    "device/ota/request",
    "esp32_ota_client",
    "http://updates.example.com/firmware.bin",
    "1.0.0"
  );
}

void loop() {
  // 讓 mqtt module 進行一次狀態機的迭代（會處理 WiFi/網路檢查/MQTT 訂閱/OTA 等）
  mqtt_state_loop_iteration();

  // 若狀態為 RUNNING，則由 main loop 處理常態運作（維持 MQTT loop / 週期 publish）
  if (mqtt_get_state() == MS_RUNNING) {
    mqtt_loop();
    // 嘗試維持連線
    //if (!mqtt_connected()) {
    //  mqtt_connect_and_subscribe();
    //  delay(200);
    //} else {
    //  mqtt_loop();
    }

    // 週期性 publish 範例（每 60s）
    static unsigned long lastPublish = 0;
    const unsigned long PUBLISH_INTERVAL_MS = 60UL * 1000UL;
    if (millis() - lastPublish >= PUBLISH_INTERVAL_MS) {
      lastPublish = millis();
      String payload = String("{\"id\":\"") + WiFi.macAddress() + String("\",\"version\":\"") + mqtt_get_current_version() + String("\"}");
      mqtt_publish(mqtt_get_pub_topic(), payload.c_str(), payload.length());
    }
  }
