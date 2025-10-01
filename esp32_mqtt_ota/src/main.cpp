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
#include "App/Utils/PageManager/PageManager.h"

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

// 觸控校準變數
static int16_t touch_x_min = 300, touch_x_max = 3800;
static int16_t touch_y_min = 300, touch_y_max = 3800;
static bool calibration_mode = false;
static int calibration_step = 0; // 0=上, 1=下, 2=左, 3=右, 4=完成
static unsigned long last_touch_time = 0;
static int16_t cal_coords[4][2]; // 儲存四個點的原始座標 [step][x,y]

// LVGL touch driver callback using LovyanGFX touch
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    uint16_t touchX, touchY;
    bool touched = tft.getTouch(&touchX, &touchY);
    
    if (touched) {
        // 校準模式：按順序收集座標
        if (calibration_mode && calibration_step < 4) {
            // 避免重複觸發
            if (millis() - last_touch_time > 1000) {
                last_touch_time = millis();
                cal_coords[calibration_step][0] = touchX;
                cal_coords[calibration_step][1] = touchY;
                
                const char* positions[] = {"TOP", "BOTTOM", "LEFT", "RIGHT"};
                Serial.printf("Step %d (%s): Raw touch: (%d,%d)\n", 
                    calibration_step + 1, positions[calibration_step], touchX, touchY);
                
                calibration_step++;
                
                // 更新螢幕顯示下一步
                if (calibration_step < 4) {
                    tft.fillScreen(TFT_BLACK);
                    tft.setTextColor(TFT_WHITE);
                    tft.setTextSize(2);
                    tft.setCursor(10, 10);
                    tft.printf("STEP %d/4", calibration_step + 1);
                    tft.setCursor(10, 40);
                    
                    switch(calibration_step) {
                        case 0: tft.println("Touch TOP"); break;
                        case 1: tft.println("Touch BOTTOM"); break;
                        case 2: tft.println("Touch LEFT"); break;
                        case 3: tft.println("Touch RIGHT"); break;
                    }
                    
                    // 顯示當前步驟的提示點
                    switch(calibration_step) {
                        case 0: // TOP
                            tft.fillCircle(160, 20, 10, TFT_RED);
                            break;
                        case 1: // BOTTOM
                            tft.fillCircle(160, 220, 10, TFT_RED);
                            break;
                        case 2: // LEFT
                            tft.fillCircle(20, 120, 10, TFT_RED);
                            break;
                        case 3: // RIGHT
                            tft.fillCircle(300, 120, 10, TFT_RED);
                            break;
                    }
                } else {
                    // 校準完成，計算校準值
                    Serial.println("=== CALIBRATION COMPLETED ===");
                    Serial.printf("TOP: (%d,%d)\n", cal_coords[0][0], cal_coords[0][1]);
                    Serial.printf("BOTTOM: (%d,%d)\n", cal_coords[1][0], cal_coords[1][1]);
                    Serial.printf("LEFT: (%d,%d)\n", cal_coords[2][0], cal_coords[2][1]);
                    Serial.printf("RIGHT: (%d,%d)\n", cal_coords[3][0], cal_coords[3][1]);
                    
                    // 計算建議的校準值
                    int16_t new_x_min = (cal_coords[2][0] + cal_coords[0][0] + cal_coords[1][0]) / 3; // LEFT的X + 上下的X平均
                    int16_t new_x_max = (cal_coords[3][0] + cal_coords[0][0] + cal_coords[1][0]) / 3; // RIGHT的X + 上下的X平均
                    int16_t new_y_min = (cal_coords[0][1] + cal_coords[2][1] + cal_coords[3][1]) / 3; // TOP的Y + 左右的Y平均
                    int16_t new_y_max = (cal_coords[1][1] + cal_coords[2][1] + cal_coords[3][1]) / 3; // BOTTOM的Y + 左右的Y平均
                    
                    Serial.println("=== SUGGESTED CALIBRATION VALUES ===");
                    Serial.printf("touch_x_min = %d;\n", new_x_min);
                    Serial.printf("touch_x_max = %d;\n", new_x_max);
                    Serial.printf("touch_y_min = %d;\n", new_y_min);
                    Serial.printf("touch_y_max = %d;\n", new_y_max);
                    Serial.println("=====================================");
                    
                    // 更新校準值
                    touch_x_min = new_x_min;
                    touch_x_max = new_x_max;
                    touch_y_min = new_y_min;
                    touch_y_max = new_y_max;
                    
                    tft.fillScreen(TFT_BLACK);
                    tft.setTextColor(TFT_GREEN);
                    tft.setTextSize(2);
                    tft.setCursor(10, 10);
                    tft.println("CALIBRATION");
                    tft.setCursor(10, 40);
                    tft.println("COMPLETED!");
                    tft.setCursor(10, 70);
                    tft.println("Check Serial");
                }
            }
            return;
        }
        
        // 正常模式：座標映射
        int16_t mappedX = map(touchX, touch_x_min, touch_x_max, 0, 319);
        int16_t mappedY = map(touchY, touch_y_min, touch_y_max, 0, 239);
        
        // 限制在螢幕範圍內
        mappedX = constrain(mappedX, 0, 319);
        mappedY = constrain(mappedY, 0, 239);
        
        data->state = LV_INDEV_STATE_PR;
        data->point.x = mappedX;
        data->point.y = mappedY;
        
        // Debug output - 顯示映射後的座標
        if (!calibration_mode) {
            Serial.printf("Touch: raw(%d,%d) -> mapped(%d,%d)\n", touchX, touchY, mappedX, mappedY);
        }
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize the display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  // Check if touch is available - 加強觸控檢測
  Serial.println("Checking touch controller...");
  if (tft.touch()) {
    Serial.println("Touch controller detected and initialized");
    Serial.println("=== TOUCH CALIBRATION MODE STARTING ===");
    Serial.println("Follow the on-screen prompts:");
    Serial.println("1. Touch TOP center");
    Serial.println("2. Touch BOTTOM center");
    Serial.println("3. Touch LEFT center");
    Serial.println("4. Touch RIGHT center");
    Serial.println("===========================================");
    
    // 啟用校準模式
    calibration_mode = true;
    calibration_step = 0;
    
  } else {
    Serial.println("Warning: Touch controller not detected");
  }

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

  // Initialize touch input device
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  // 延遲初始化 PageManager，讓校準模式先完成
  if (!calibration_mode) {
    // Initialize PageManager with MVP architecture
    pageManager = PageManager::getInstance();
    if (pageManager) {
      pageManager->init();
      Serial.println("PageManager with MVP architecture initialized");
      Serial.printf("Current page: %s\n", pageManager->getPageName(pageManager->getCurrentPage()));
    } else {
      Serial.println("Failed to initialize PageManager");
    }
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
  
  // 校準模式控制 - 完成4步後自動關閉
  if (calibration_mode && calibration_step >= 4) {
    // 等待3秒後關閉校準模式
    static unsigned long completion_time = millis();
    if (millis() - completion_time > 3000) {
      calibration_mode = false;
      Serial.println("=== CALIBRATION MODE DISABLED ===");
      Serial.println("Switching to normal operation...");
      
      // 清除螢幕
      tft.fillScreen(TFT_BLACK);
      
      // 現在初始化 PageManager
      if (pageManager == nullptr) {
        pageManager = PageManager::getInstance();
        if (pageManager) {
          pageManager->init();
          Serial.println("PageManager with MVP architecture initialized");
          Serial.printf("Current page: %s\n", pageManager->getPageName(pageManager->getCurrentPage()));
        } else {
          Serial.println("Failed to initialize PageManager");
        }
      }
    }
  }
  
  // 校準模式：顯示視覺引導
  if (calibration_mode && calibration_step < 4) {
    static bool visual_guide_shown = false;
    if (!visual_guide_shown) {
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_WHITE);
      tft.setTextSize(2);
      tft.setCursor(10, 10);
      tft.printf("STEP %d/4", calibration_step + 1);
      tft.setCursor(10, 40);
      
      switch(calibration_step) {
        case 0: 
          tft.println("Touch TOP");
          tft.fillCircle(160, 20, 10, TFT_RED);
          break;
        case 1: 
          tft.println("Touch BOTTOM");
          tft.fillCircle(160, 220, 10, TFT_RED);
          break;
        case 2: 
          tft.println("Touch LEFT");
          tft.fillCircle(20, 120, 10, TFT_RED);
          break;
        case 3: 
          tft.println("Touch RIGHT");
          tft.fillCircle(300, 120, 10, TFT_RED);
          break;
      }
      visual_guide_shown = true;
    }
  } else {
    static bool visual_guide_shown = false;
    visual_guide_shown = false; // 重置標誌
  }
  
  // 觸控測試 - 只在非校準模式下每2秒檢查一次
  static unsigned long lastTouchTest = 0;
  if (!calibration_mode && millis() - lastTouchTest >= 2000) {
    lastTouchTest = millis();
    uint16_t x, y;
    if (tft.getTouch(&x, &y)) {
      Serial.printf("Direct touch test: (%d,%d)\n", x, y);
    }
  }
  
  // 頁面切換演示 (每30秒切換一次頁面) - 現在可以用觸控覆蓋
  static unsigned long lastPageSwitch = 0;
  const unsigned long PAGE_SWITCH_INTERVAL_MS = 30UL * 1000UL;
  if (!calibration_mode && millis() - lastPageSwitch >= PAGE_SWITCH_INTERVAL_MS) {
    lastPageSwitch = millis();
    if (pageManager) {
      pageManager->switchToNextPage();
      Serial.printf("Auto-switched to page: %s\n", pageManager->getPageName(pageManager->getCurrentPage()));
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