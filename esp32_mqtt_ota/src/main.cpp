/*
  ESP32 OLED Display Application with 3-Button Control (HAL Integrated)
  
  This application provides a 128x64 OLED display interface using LVGL with LovyanGFX.
  Features include 3-button navigation, page management with MVP architecture, and visual display.
  The application supports multiple pages (Menu, Settings, Status) with button control.
  
  Hardware abstraction layer (HAL) manages all hardware modules:
  - Display: SSD1306 128x64 OLED (I2C: SDA=21, SCL=22, Addr=0x3C)  
  - Buttons: GPIO 32 (Menu/OK), GPIO 33 (UP/Back), GPIO 34 (Down/Fn)
  - SA818: Radio transceiver module
  - Clock: Time management with Taiwan timezone
  - Power: Battery monitoring and power management
  
  注意: GPIO 34需要外部10kΩ上拉電阻連接到3.3V
  
  Display Layout:
  - Top 16px (Yellow zone): Page titles and status
  - Bottom 48px (Blue zone): Page content
*/

#include <Arduino.h>
#include <lvgl.h>

// HAL System
#include "App/Common/HAL/HAL.h"
#include "App/Common/DataProc/DataProc.h"

// Application Framework
#include "App/Utils/PageManager/PageManager.h"
#include "App/Pages/Status/StatusView.h"
#include "App/Pages/Status/StatusPresenter.h"
#include "App/Pages/Menu/MenuPresenter.h"
#include "App/Pages/Menu/MenuView.h"
#include "App/Pages/Trekking/TrekkingView.h"
#include "App/Pages/WalkieTalkie/WalkieTalkieView.h"
#include "App/Pages/System/SystemView.h"

// OTA Support
#include "App/Utils/OTA/ota_updater.h"
// WiFi Manager
#include "App/Utils/WiFiManager/wifi_manager.h"

// Application state
PageManager* pageManager = nullptr;
static WiFiManager wifiManager; // WiFi Manager instance

// OTA Configuration is now in ota_config.h

// LVGL display buffer for 128x64 OLED
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[LVGL_BUFFER_SIZE]; // Buffer defined in HAL_Config.h

// LVGL display driver callback using HAL
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    HAL::Display_SetAddrWindow(area->x1, area->y1, area->x2, area->y2);
    HAL::Display_SendPixels((uint16_t*)&color_p->full, w * h);

    lv_disp_flush_ready(disp);
}

// Helper function for time and battery status using HAL
void getTimeAndBattery(char* timeStr, char* battStr) {
    char rawBattStr[16];
    HAL::Clock_GetTimeString(timeStr, rawBattStr);
    
    // 只顯示百分比，移除 "Batt" 和其他前綴
    // 假設 rawBattStr 可能是 "Batt:85%" 或 "85%" 格式
    const char* percentPos = strstr(rawBattStr, "%");
    if (percentPos) {
        // 找到百分比符號，向前查找數字
        const char* numStart = rawBattStr;
        while (*numStart && !isdigit(*numStart)) {
            numStart++;
        }
        if (*numStart) {
            snprintf(battStr, 16, "%s", numStart);
        } else {
            strcpy(battStr, "85%");  // 預設值
        }
    } else {
        strcpy(battStr, "85%");  // 預設值
    }
}

// 簡化的設定變數 - 移除觸控功能

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("=== ESP32 OLED Application with HAL System ===");
  
  #ifdef ENABLE_AUTO_OTA_CHECK
  Serial.println("===== WiFi Manager Debug =====");
  
  // 初始化 WiFi Manager
  wifiManager.begin();
  
  // 嘗試自動連接到保存的 WiFi
  bool connected = wifiManager.autoConnectToWiFi();
  Serial.printf("WiFi connection result: %s\n", connected ? "Success" : "Failed");
  
  if (connected) {
      Serial.println("Connected to WiFi network!");
      Serial.print("IP address: ");
      Serial.println(wifiManager.getIP());
      Serial.printf("Connected to SSID: %s\n", wifiManager.getSSID().c_str());
  } else {
      Serial.println("Failed to connect to saved WiFi.");
      Serial.println("Starting WiFi configuration portal...");
      wifiManager.startConfigPortal();
      Serial.println("OTA functionality will be available after WiFi setup.");
  }
  Serial.println("===== WiFi Manager Debug End =====");
  #endif

  #ifdef OTA_SERVER_URL
  String serverURL = OTA_SERVER_URL;
  #else
  String serverURL = "http://your-server.com/firmware";
  #endif
  
  #ifdef OTA_VERSION_URL
  String versionURL = OTA_VERSION_URL;
  #else
  String versionURL = "http://your-server.com/version";
  #endif
  
  #ifdef OTA_CHECK_INTERVAL
  unsigned long interval = OTA_CHECK_INTERVAL;
  #else
  unsigned long interval = 3600; // 1 hour default
  #endif

  // Initialize HAL System (handles all hardware)
  HAL::HAL_Init();
  delay(1000);

  // Initialize LVGL
  Serial.println("Initializing LVGL...");
  lv_init();
  
  // 設定全域文字渲染優化
  lv_style_t global_style;
  lv_style_init(&global_style);
  lv_style_set_text_opa(&global_style, LV_OPA_COVER);
  lv_style_set_text_line_space(&global_style, 1);
  lv_style_set_text_letter_space(&global_style, 0);

  // Initialize display buffer using HAL configuration
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, LVGL_BUFFER_SIZE);
  
  // Initialize display driver using HAL
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = DISPLAY_WIDTH;   // From HAL_Config.h
  disp_drv.ver_res = DISPLAY_HEIGHT;  // From HAL_Config.h
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // 設定全局樣式以改善文字渲染品質
  static lv_style_t global_text_style;
  lv_style_init(&global_text_style);
  lv_style_set_text_opa(&global_text_style, LV_OPA_COVER);
  lv_style_set_text_font(&global_text_style, &lv_font_unscii_8);
  
  Serial.println("LVGL initialized with HAL integration");

  // Initialize DataProc System after LVGL (requires lv_mem_alloc)
  Serial.println("Initializing DataProc System...");
  Serial.print("Free heap before DataProc init: ");
  Serial.println(ESP.getFreeHeap());
  
  DataProc_Init();
  
  Serial.print("Free heap after DataProc init: ");
  Serial.println(ESP.getFreeHeap());
  Serial.println("DataProc System initialized");

  // Initialize PageManager with MVP architecture
  pageManager = PageManager::getInstance();
  if (pageManager) {
    pageManager->init();
    Serial.println("PageManager with MVP architecture initialized");
    Serial.printf("Current page: %s\n", pageManager->getPageName(pageManager->getCurrentPage()));
  } else {
    Serial.println("Failed to initialize PageManager");
  }
  
  #ifdef ENABLE_AUTO_OTA_CHECK
  // Initialize OTA if WiFi is connected
  if (wifiManager.isConnected()) {
    Serial.println("=== OTA Initialization ===");
    
    #ifdef CURRENT_VERSION
    String currentVersion = CURRENT_VERSION;
    #else
    String currentVersion = "1.0.0";
    #endif
    
    // Initialize OTA updater
    otaUpdater.begin(currentVersion, serverURL, versionURL, interval);
    otaUpdater.enableAutoCheck(true);
    
    // Perform initial update check
    Serial.println("Performing initial OTA check...");
    if (otaUpdater.checkForUpdates()) {
      Serial.println("Update available! Will update on next check cycle.");
    } else {
      Serial.println("No updates available.");
    }
    
    Serial.println("=== OTA Initialization Complete ===");
  } else {
    Serial.println("WiFi not connected - OTA disabled");
  }
  #endif

  // Test SA818 module after initialization
  Serial.println("=== SA818 Module Test ===");
  delay(2000);  // Give SA818 time to initialize
  
  // Call SA818 scan function to test communication
  Serial.println("Testing SA818 communication...");
  HAL::SA818_scan();
  
  Serial.println("=== Setup Complete - HAL System Ready ===");
}

void loop() {
  // Handle LVGL tasks
  lv_timer_handler();
  
  #ifdef ENABLE_AUTO_OTA_CHECK
  // Handle WiFi Manager portal
  wifiManager.loop();
  
  // Handle automatic OTA checks (only if WiFi is connected)
  if (wifiManager.isConnected()) {
    otaUpdater.handleAutoCheck();
  }
  #endif
  
  // Handle button events using HAL
  HAL::Button_HandleEvents();
  
  // 更新狀態頁面系統信息（每2秒更新一次）
  static unsigned long lastStatusUpdate = 0;
  if (millis() - lastStatusUpdate >= STATUS_UPDATE_INTERVAL) {
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
  if (millis() - lastTimeUpdate >= TIME_UPDATE_INTERVAL) {
    lastTimeUpdate = millis();
    if (pageManager) {
      PageID currentPage = pageManager->getCurrentPage();
      
      // 獲取台灣時間和電池狀態 (使用HAL)
      char timeStr[16], battStr[16];
      getTimeAndBattery(timeStr, battStr);
      
      switch (currentPage) {
        case PAGE_MAINMENU:
          if (pageManager->getMenuPresenter()) {
            pageManager->getMenuPresenter()->updateDisplay();
          }
          // 更新 MainMenu 狀態欄 (時間+電池)
          if (pageManager->getMenuView()) {
            pageManager->getMenuView()->updateStatusBar(battStr, timeStr);
          }
          break;
          
        case PAGE_TREKKING:
          if (pageManager->getTrekkingView()) {
            pageManager->getTrekkingView()->updateStatusBar(battStr, timeStr);
            
            // 模擬環境數據更新（實際應用中這些數據來自感測器）
            static unsigned long lastTrekkingUpdate = 0;
            if (millis() - lastTrekkingUpdate > 1000) {  // 每秒更新一次
              // 模擬溫度變化 (24-27°C)
              float temp = 25.5 + sin(millis() / 10000.0) * 1.5;
              // 模擬海拔變化 (500-550m)
              float alt = 520 + sin(millis() / 8000.0) * 30;
              // 模擬氣壓變化 (1010-1015 hPa)
              float press = 1012 + sin(millis() / 12000.0) * 3;
              
              pageManager->getTrekkingView()->updateEnvironmentData(temp, alt, press);
              
              // 如果在 RUNNING 狀態，更新累計數據
              if (pageManager->getTrekkingView()->getState() == TrekkingState::RUNNING) {
                unsigned long elapsedSec = (millis() / 1000) % 3600; // 模擬1小時內的時間
                float distance = elapsedSec * 0.001f; // 模擬距離增長
                float ascent = elapsedSec * 0.2f; // 模擬爬升
                int steps = elapsedSec * 2; // 模擬步數
                
                pageManager->getTrekkingView()->updateCumulativeData(elapsedSec, distance, ascent, steps);
              }
              
              lastTrekkingUpdate = millis();
            }
          }
          break;
          
        case PAGE_WALKIETALKIE:
          if (pageManager->getWalkieTalkieView()) {
            pageManager->getWalkieTalkieView()->updateStatusBar(battStr, timeStr);
            
            // 模擬無線電 RSSI 數據更新
            static unsigned long lastRadioUpdate = 0;
            if (millis() - lastRadioUpdate > 2000) {  // 每2秒更新一次
              RadioStatus currentStatus = pageManager->getWalkieTalkieView()->getRadioStatus();
              
              // 模擬 RSSI 變化 (-90dBm 到 -70dBm)
              currentStatus.rssi = -85 + (int)(sin(millis() / 5000.0) * 10);
              
              // 模擬 RX/TX 狀態變化
              static int rxTxCounter = 0;
              if (rxTxCounter % 10 == 0) {
                currentStatus.rxTxMode = "TX";
              } else if (rxTxCounter % 10 == 1) {
                currentStatus.rxTxMode = "RX";
              } else {
                currentStatus.rxTxMode = "RX";
              }
              rxTxCounter++;
              
              pageManager->getWalkieTalkieView()->updateRadioStatus(currentStatus);
              lastRadioUpdate = millis();
            }
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
  
  // Handle OTA automatic checks
  #ifdef ENABLE_AUTO_OTA_CHECK
  otaUpdater.handleAutoCheck();
  #endif
  
  // Small delay to prevent watchdog issues
  delay(5);
}