/*
  ESP32 OLED Display Application with 3-Button Control
  
  This application provides a 128x64 OLED display interface using LVGL with LovyanGFX.
  Features include 3-button navigation, page management with MVP architecture, and visual display.
  The application supports multiple pages (Menu, Settings, Status) with button control.
  
  Hardware Configuration:
  - Display: SSD1306 128x64 OLED (I2C: SDA=21, SCL=22, Addr=0x3C)
  - Button 1: GPIO 32 (Menu/OK) - Short: Menu, Long: Settings
  - Button 2: GPIO 33 (UP/Back) - Short: Previous page, Long: Status  
  - Button 3: GPIO 34 (Down/Fn) - Short: Next page, Long: Reserved, Double: Function
  
  注意: GPIO 34需要外部10kΩ上拉電阻連接到3.3V
  
  Display Layout:
  - Top 16px (Yellow zone): Page titles and status
  - Bottom 48px (Blue zone): Page content
*/

#include <Arduino.h>
#include <lvgl.h>
#include <LovyanGFX.hpp>
#include "LGFX_ChappieCore.hpp"
#include "App/Utils/PageManager/PageManager.h"
#include "App/Utils/ButtonManager.h"
#include "App/Pages/Status/StatusView.h"
#include "App/Pages/Status/StatusPresenter.h"
#include "App/Pages/Settings/SettingsView.h"
#include "App/Pages/Settings/SettingsPresenter.h"
#include "App/Pages/Menu/MenuPresenter.h"

static LGFX tft; 
static PageManager* pageManager = nullptr;
static ButtonManager* buttonManager = nullptr;

// LVGL display buffer for 128x64 OLED
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[128 * 8]; // 8 lines buffer for 128x64 OLED

// LVGL display driver callback
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushPixels((uint16_t*)&color_p->full, w * h);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

// 按鍵事件處理函數
void handleButtonEvents() {
  if (!pageManager || !buttonManager) return;
  
  PageID currentPage = pageManager->getCurrentPage();
  MenuPresenter* menuPresenter = pageManager->getMenuPresenter();
  
  // 檢查Menu/OK按鍵 (GPIO 32)
  ButtonEvent menuEvent = buttonManager->getButtonEvent(ButtonManager::BTN_MENU_OK);
  if (menuEvent == BTN_EVENT_PRESS) {
    Serial.println("Menu/OK button pressed");
    
    if (currentPage == PAGE_MENU && menuPresenter) {
      // 在Menu頁面：執行選中的項目
      menuPresenter->executeSelectedItem();
      int selectedId = menuPresenter->getCurrentSelection();
      
      // 根據選中項目切換到對應頁面
      switch (selectedId) {
        case 0: // Settings項目 -> Settings頁面
          Serial.println("Menu: Settings selected -> Going to Settings page");
          pageManager->switchToPage(PAGE_SETTINGS);
          break;
        case 1: // Status項目 -> Status頁面
          Serial.println("Menu: Status selected -> Going to Status page");
          pageManager->switchToPage(PAGE_STATUS);
          break;
      }
    } else {
      // 其他頁面：短按進入Menu頁面
      pageManager->switchToPage(PAGE_MENU);
    }
  } else if (menuEvent == BTN_EVENT_HOLD) {
    Serial.println("Menu/OK button hold - entering settings");
    // 長按：直接進入設定頁面
    pageManager->switchToPage(PAGE_SETTINGS);
  }
  
  // 檢查UP/Back按鍵 (GPIO 33)
  ButtonEvent upEvent = buttonManager->getButtonEvent(ButtonManager::BTN_UP_BACK);
  if (upEvent == BTN_EVENT_PRESS) {
    Serial.println("UP/Back button pressed");
    
    if (currentPage == PAGE_MENU && menuPresenter) {
      // 在Menu頁面：向上移動選擇
      menuPresenter->moveUp();
    } else if (currentPage == PAGE_STATUS && pageManager->getStatusPresenter()) {
      // 在Status頁面：向上滾動內容
      pageManager->getStatusPresenter()->scrollUp();
    } else if (currentPage == PAGE_SETTINGS && pageManager->getSettingsPresenter()) {
      // 在Settings頁面：向上滾動內容
      pageManager->getSettingsPresenter()->scrollUp();
    } else {
      // 其他頁面：返回上一頁
      pageManager->switchToPreviousPage();
    }
  } else if (upEvent == BTN_EVENT_HOLD) {
    Serial.println("UP/Back button hold - going to status");
    // 長按：直接進入狀態頁面
    pageManager->switchToPage(PAGE_STATUS);
  }
  
  // 檢查Down/Fn按鍵 (GPIO 34)
  ButtonEvent downEvent = buttonManager->getButtonEvent(ButtonManager::BTN_DOWN_FN);
  if (downEvent == BTN_EVENT_PRESS) {
    Serial.println("Down/Fn button pressed");
    
    if (currentPage == PAGE_MENU && menuPresenter) {
      // 在Menu頁面：向下移動選擇
      menuPresenter->moveDown();
    } else if (currentPage == PAGE_STATUS && pageManager->getStatusPresenter()) {
      // 在Status頁面：向下滾動內容
      pageManager->getStatusPresenter()->scrollDown();
    } else if (currentPage == PAGE_SETTINGS && pageManager->getSettingsPresenter()) {
      // 在Settings頁面：向下滾動內容
      pageManager->getSettingsPresenter()->scrollDown();
    } else {
      // 其他頁面：切換到下一頁
      pageManager->switchToNextPage();
    }
  } else if (downEvent == BTN_EVENT_HOLD) {
    Serial.println("Down/Fn button hold - reserved function");
    // 長按：保留功能（可以分配給其他用途）
  } else if (downEvent == BTN_EVENT_DOUBLE) {
    Serial.println("Down/Fn button double click - function mode");
    // 雙擊：特殊功能模式 - 可以在這裡添加特定功能
    // 注意：不執行清屏操作，避免螢幕消失
    Serial.println("Function mode activated (placeholder for future features)");
  }
}

// 簡化的設定變數 - 移除觸控功能

void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize the OLED display
  Serial.println("正在初始化 OLED 顯示器...");
  Serial.println("I2C 配置: SDA=21, SCL=22, Address=0x3C");
  
  bool initResult = tft.init();
  if (initResult) {
    Serial.println("✅ LGFX 初始化成功");
  } else {
    Serial.println("❌ LGFX 初始化失敗 - 請檢查硬體連接");
  }
  
  tft.setRotation(0); // OLED通常使用0度旋轉
  tft.fillScreen(TFT_BLACK);
  
  // 測試顯示器是否工作
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setCursor(0, 0);
  tft.println("ESP32 OLED Test");
  tft.println("SSD1306 128x64");
  
  Serial.printf("顯示器尺寸: %dx%d\n", tft.width(), tft.height());
  Serial.println("SSD1306 128x64 OLED Display initialized");

  // Initialize Button Manager
  buttonManager = ButtonManager::getInstance();
  buttonManager->init();

  // Initialize LVGL
  lv_init();
  
  // 設定全域文字渲染優化
  lv_style_t global_style;
  lv_style_init(&global_style);
  lv_style_set_text_opa(&global_style, LV_OPA_COVER);
  lv_style_set_text_line_space(&global_style, 1);
  lv_style_set_text_letter_space(&global_style, 0);

  // Initialize display buffer for 128x64 OLED
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, 128 * 8);
  
  // Initialize display driver for OLED
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 128;  // OLED寬度
  disp_drv.ver_res = 64;   // OLED高度
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // 設定全局樣式以改善文字渲染品質
  static lv_style_t global_text_style;
  lv_style_init(&global_text_style);
  lv_style_set_text_opa(&global_text_style, LV_OPA_COVER);
  lv_style_set_text_font(&global_text_style, &lv_font_montserrat_14);
  
  // 無觸控輸入設備 - OLED顯示器不需要觸控

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

void loop() {
  // Handle LVGL tasks
  lv_timer_handler();
  
  // Update button manager
  if (buttonManager) {
    buttonManager->update();
    
    // Handle button events
    handleButtonEvents();
  }
  
  // 自動頁面切換已禁用（原始為每30秒自動切換）。
  // 如果需要重新啟用自動切換，可恢復原始邏輯或設置一個配置標誌。
  // Serial.println("Auto page switching disabled");
  
  // 更新狀態頁面系統信息（每2秒更新一次）
  static unsigned long lastStatusUpdate = 0;
  const unsigned long STATUS_UPDATE_INTERVAL_MS = 2000UL; // 2秒
  
  if (millis() - lastStatusUpdate >= STATUS_UPDATE_INTERVAL_MS) {
    lastStatusUpdate = millis();
    if (pageManager && pageManager->getStatusPresenter()) {
      // 如果當前在狀態頁面，則更新系統狀態
      if (pageManager->getCurrentPage() == PAGE_STATUS) {
        pageManager->getStatusPresenter()->updateStatus();
      }
    }
  }
  
  // Small delay to prevent watchdog issues
  delay(5);
}