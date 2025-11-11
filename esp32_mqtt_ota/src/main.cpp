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
#include <time.h>
#include <sys/time.h>
#include "App/Utils/PageManager/PageManager.h"
#include "App/Utils/ButtonManager.h"
#include "App/Pages/Status/StatusView.h"
#include "App/Pages/Status/StatusPresenter.h"
// Settings頁面已移除
// #include "App/Pages/Settings/SettingsView.h"
// #include "App/Pages/Settings/SettingsPresenter.h"
#include "App/Pages/Menu/MenuPresenter.h"
#include "App/Pages/Trekking/TrekkingView.h"
#include "App/Pages/WalkieTalkie/WalkieTalkieView.h"
#include "App/Pages/System/SystemView.h"

static LGFX tft; 
static PageManager* pageManager = nullptr;
static ButtonManager* buttonManager = nullptr;
static bool scrollMode = false; // 滾動模式狀態

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

// 台灣時間工具函數
void getTaiwanTime(char* timeStr, char* battStr) {
    struct tm timeinfo;
    time_t now = time(nullptr);
    localtime_r(&now, &timeinfo);
    
    // 檢查時間是否有效
    if (timeinfo.tm_year < 125 || timeinfo.tm_year > 200) {
        // 時間無效，使用預設時間
        strcpy(timeStr, "14:30");
    } else {
        // 格式化時間 (24小時制，台灣習慣)
        strftime(timeStr, 16, "%H:%M", &timeinfo);
    }
    
    // Simulate battery level (85-99%)
    int battPercent = 85 + (millis() / 10000) % 15;
    snprintf(battStr, 16, "Batt:%d%%", battPercent);
}

// 按鍵事件處理函數
void handleButtonEvents() {
  if (!pageManager || !buttonManager) return;
  
  PageID currentPage = pageManager->getCurrentPage();
  MenuPresenter* menuPresenter = pageManager->getMenuPresenter();
  
  // 檢查Menu/OK按鍵 (GPIO 32)
  ButtonEvent menuEvent = buttonManager->getButtonEvent(ButtonManager::BTN_MENU_OK);
  if (menuEvent == BTN_EVENT_PRESS) {
    Serial.printf("Menu/OK button pressed (Current page: %d)\n", (int)currentPage);
    
    // 根據當前頁面執行不同的OK按鍵功能
    switch (currentPage) {
      case PAGE_MAINMENU:
        Serial.println("MainMenu: OK button - No action");
        // 在 MainMenu 頁面，OK 按鍵無作用
        break;
        
      case PAGE_TREKKING:
        Serial.println("Trekking: OK button - Toggle scroll mode");
        scrollMode = !scrollMode;
        Serial.printf("Scroll mode: %s\n", scrollMode ? "ON" : "OFF");
        break;
        
      case PAGE_WALKIETALKIE:
        Serial.println("WalkieTalkie: OK button - Toggle scroll mode");
        scrollMode = !scrollMode;
        Serial.printf("Scroll mode: %s\n", scrollMode ? "ON" : "OFF");
        break;
        
      case PAGE_STATUS:
        Serial.println("Status: OK button - Toggle scroll mode");
        scrollMode = !scrollMode;
        Serial.printf("Scroll mode: %s\n", scrollMode ? "ON" : "OFF");
        break;
        
      case PAGE_SYSTEM:
        Serial.println("System: OK button - Toggle scroll mode");
        scrollMode = !scrollMode;
        Serial.printf("Scroll mode: %s\n", scrollMode ? "ON" : "OFF");
        break;
        
      default:
        Serial.printf("OK button ignored (Page: %d)\n", (int)currentPage);
        break;
    }
  } else if (menuEvent == BTN_EVENT_HOLD) {
    Serial.printf("Menu/OK button hold 2sec (Current page: %d)\n", (int)currentPage);
    
    // 任何頁面長按OK鍵2秒：返回MainMenu
    if (currentPage != PAGE_MAINMENU) {
      Serial.println("Any page: OK hold 2sec -> MainMenu");
      scrollMode = false; // 重置滾動模式
      pageManager->switchToPage(PAGE_MAINMENU);
    } else {
      Serial.println("MainMenu: OK hold - No action (already on MainMenu)");
    }
  }
  
  // 檢查UP按鍵 (GPIO 33)
  ButtonEvent upEvent = buttonManager->getButtonEvent(ButtonManager::BTN_UP_BACK);
  if (upEvent == BTN_EVENT_PRESS) {
    if (currentPage == PAGE_MAINMENU) {
      // MainMenu頁面：UP按鍵直接跳轉到 Trekking
      Serial.println("UP button pressed - MainMenu -> Trekking");
      pageManager->switchToPage(PAGE_TREKKING);
    } else if (scrollMode) {
      // 滾動模式：UP按鍵用於螢幕向上滾動
      Serial.println("UP button pressed - Screen scroll up");
      switch (currentPage) {
        case PAGE_TREKKING:
          if (pageManager->getTrekkingView()) {
            pageManager->getTrekkingView()->scrollUp();
          }
          break;
        case PAGE_WALKIETALKIE:
          if (pageManager->getWalkieTalkieView()) {
            pageManager->getWalkieTalkieView()->scrollUp();
          }
          break;
        case PAGE_STATUS:
          if (pageManager->getStatusPresenter()) {
            pageManager->getStatusPresenter()->scrollUp();
          }
          break;
        case PAGE_SYSTEM:
          if (pageManager->getSystemView()) {
            pageManager->getSystemView()->scrollUp();
          }
          break;
        default:
          break;
      }
    } else {
      // 非滾動模式：UP按鍵用於頁面切換
      PageID nextPage;
      switch (currentPage) {
        case PAGE_TREKKING:
          nextPage = PAGE_WALKIETALKIE;
          Serial.println("UP button pressed - Trekking -> WalkieTalkie");
          break;
        case PAGE_WALKIETALKIE:
          nextPage = PAGE_STATUS;
          Serial.println("UP button pressed - WalkieTalkie -> Status");
          break;
        case PAGE_STATUS:
          nextPage = PAGE_SYSTEM;
          Serial.println("UP button pressed - Status -> System");
          break;
        case PAGE_SYSTEM:
          nextPage = PAGE_MAINMENU;
          Serial.println("UP button pressed - System -> MainMenu");
          break;
        default:
          nextPage = PAGE_MAINMENU;
          break;
      }
      pageManager->switchToPage(nextPage);
      scrollMode = false; // 切換頁面時重置滾動模式
    }
  }
  
  // 檢查DOWN按鍵 (GPIO 34)
  ButtonEvent downEvent = buttonManager->getButtonEvent(ButtonManager::BTN_DOWN_FN);
  if (downEvent == BTN_EVENT_PRESS) {
    if (currentPage == PAGE_MAINMENU) {
      // MainMenu頁面：DOWN按鍵直接跳轉到 System
      Serial.println("DOWN button pressed - MainMenu -> System");
      pageManager->switchToPage(PAGE_SYSTEM);
    } else if (scrollMode) {
      // 滾動模式：DOWN按鍵用於螢幕向下滾動
      Serial.println("DOWN button pressed - Screen scroll down");
      switch (currentPage) {
        case PAGE_TREKKING:
          if (pageManager->getTrekkingView()) {
            pageManager->getTrekkingView()->scrollDown();
          }
          break;
        case PAGE_WALKIETALKIE:
          if (pageManager->getWalkieTalkieView()) {
            pageManager->getWalkieTalkieView()->scrollDown();
          }
          break;
        case PAGE_STATUS:
          if (pageManager->getStatusPresenter()) {
            pageManager->getStatusPresenter()->scrollDown();
          }
          break;
        case PAGE_SYSTEM:
          if (pageManager->getSystemView()) {
            pageManager->getSystemView()->scrollDown();
          }
          break;
        default:
          break;
      }
    } else {
      // 非滾動模式：DOWN按鍵用於頁面切換
      PageID prevPage;
      switch (currentPage) {
        case PAGE_TREKKING:
          prevPage = PAGE_MAINMENU;
          Serial.println("DOWN button pressed - Trekking -> MainMenu");
          break;
        case PAGE_WALKIETALKIE:
          prevPage = PAGE_TREKKING;
          Serial.println("DOWN button pressed - WalkieTalkie -> Trekking");
          break;
        case PAGE_STATUS:
          prevPage = PAGE_WALKIETALKIE;
          Serial.println("DOWN button pressed - Status -> WalkieTalkie");
          break;
        case PAGE_SYSTEM:
          prevPage = PAGE_STATUS;
          Serial.println("DOWN button pressed - System -> Status");
          break;
        default:
          prevPage = PAGE_MAINMENU;
          break;
      }
      pageManager->switchToPage(prevPage);
      scrollMode = false; // 切換頁面時重置滾動模式
    }
  } else if (downEvent == BTN_EVENT_HOLD) {
    Serial.println("Down button hold - reserved function");
    // 長按：保留功能（可以分配給其他用途）
  } else if (downEvent == BTN_EVENT_DOUBLE) {
    Serial.println("Down button double click - function mode");
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
  lv_style_set_text_font(&global_text_style, &lv_font_unscii_8);
  
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
  
  // 更新所有頁面的時間和電池顯示（每5秒更新一次）
  static unsigned long lastTimeUpdate = 0;
  const unsigned long TIME_UPDATE_INTERVAL_MS = 5000UL; // 5秒
  
  if (millis() - lastTimeUpdate >= TIME_UPDATE_INTERVAL_MS) {
    lastTimeUpdate = millis();
    if (pageManager) {
      PageID currentPage = pageManager->getCurrentPage();
      
      // 獲取台灣時間和電池狀態
      char timeStr[16], battStr[16];
      getTaiwanTime(timeStr, battStr);
      
      switch (currentPage) {
        case PAGE_MAINMENU:
          if (pageManager->getMenuPresenter()) {
            pageManager->getMenuPresenter()->updateDisplay();
          }
          break;
          
        case PAGE_TREKKING:
          if (pageManager->getTrekkingView()) {
            pageManager->getTrekkingView()->updateStatusBar(battStr, timeStr);
          }
          break;
          
        case PAGE_WALKIETALKIE:
          if (pageManager->getWalkieTalkieView()) {
            pageManager->getWalkieTalkieView()->updateStatusBar(battStr, timeStr);
          }
          break;
          
        case PAGE_STATUS:
          if (pageManager->getStatusPresenter()) {
            pageManager->getStatusPresenter()->updateStatusBar(battStr, timeStr);
          }
          break;
          
        case PAGE_SYSTEM:
          if (pageManager->getSystemView()) {
            pageManager->getSystemView()->updateStatusBar(battStr, timeStr);
          }
          break;
      }
    }
  }
  
  // Small delay to prevent watchdog issues
  delay(5);
}