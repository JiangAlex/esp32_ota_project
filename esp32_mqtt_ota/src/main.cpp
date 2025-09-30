/*
  Minimal state-machine implementation driven by docs/state_diagram.mmd
  (state-loop moved into mqtt_client module)

main.cpp 現在只留 setup() 與 loop()，loop() 中包含 RUNNING 狀態的常態處理（維持 mqtt 連線、mqtt_loop()、以及週期 publish）。
mqtt_client.cpp 含完整的狀態機與 WiFi/MQTT/OTA 邏輯（已移除 main 中的輔助函式）。
若需調整 WiFi/MQTT/OTA 的參數，請修改 setup() 內呼叫 mqtt_module_setup(...) 的字串參數。
*/

#include <Arduino.h>
#include "mqtt_client.h"
#include <lvgl.h>
#include <LovyanGFX.hpp>
#include "LGFX_ChappieCore.hpp"
//#include "App/Utils/PageManager.h"
#include "App/Utils/PageManager/PageManager.h"  // 更新包含路徑

static LGFX tft; 
static PageManager* pageManager = nullptr;

// LVGL display buffer
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[320 * 5]; // 5 lines buffer

// LVGL display driver callback
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t*)&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize the display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Initialize LVGL
  lv_init();

  // Initialize display buffer
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, 320 * 5);
  
  // Initialize display driver
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 320;
  disp_drv.ver_res = 240;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // Initialize PageManager with MVP architecture
  pageManager = PageManager::getInstance();
  if (pageManager) {
    pageManager->init();
    Serial.println("PageManager with MVP architecture initialized");
    Serial.printf("Current page: %s\n", pageManager->getPageName(pageManager->getCurrentPage()));
  } else {
    Serial.println("Failed to initialize PageManager");
  }

  // 初始化 mqtt module (包含 WiFi/MQTT/OTA 等邏輯設定)
  mqtt_module_setup(
    "ACC_TPE_WIFI_8F",
    "87738500",
    "192.168.131.55",
    1883,
    "device/ota/cmd",
    "device/ota/request",
    "esp32_ota_client",
    "http://updates.example.com/firmware.bin",
    "1.0.0"
  );
}

void loop() {
  // Handle LVGL tasks
  lv_timer_handler();
  
  // 頁面切換演示 (每30秒切換一次頁面)
  static unsigned long lastPageSwitch = 0;
  const unsigned long PAGE_SWITCH_INTERVAL_MS = 30UL * 1000UL;
  if (millis() - lastPageSwitch >= PAGE_SWITCH_INTERVAL_MS) {
    lastPageSwitch = millis();
    if (pageManager) {
      pageManager->switchToNextPage();
      Serial.printf("Switched to page: %s\n", pageManager->getPageName(pageManager->getCurrentPage()));
    }
  }
  
  // 讓 mqtt module 進行一次狀態機的迭代（會處理 WiFi/網路檢查/MQTT 訂閱/OTA 等）
  mqtt_state_loop_iteration();

  // 若狀態為 RUNNING，則由 main loop 處理常態運作（維持 MQTT loop / 週期 publish）
  if (mqtt_get_state() == MS_RUNNING) {
    mqtt_loop();

    // 週期性 publish 範例（每 60s）
    static unsigned long lastPublish = 0;
    const unsigned long PUBLISH_INTERVAL_MS = 60UL * 1000UL;
    if (millis() - lastPublish >= PUBLISH_INTERVAL_MS) {
      lastPublish = millis();
      String payload = String("{\"id\":\"") + WiFi.macAddress() + String("\",\"version\":\"") + mqtt_get_current_version() + String("\"}");
      mqtt_publish(mqtt_get_pub_topic(), payload.c_str(), payload.length());
    }
  }
  
  // Small delay to prevent watchdog issues
  delay(5);
}