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

// Application state
static PageManager* pageManager = nullptr;
static bool scrollMode = false; // 滾動模式狀態

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

// 按鍵事件處理函數 (使用HAL系統)
void handleButtonEvents() {
  if (!pageManager) return;
  
  PageID currentPage = pageManager->getCurrentPage();
  MenuPresenter* menuPresenter = pageManager->getMenuPresenter();
  
  // 檢查Menu/OK按鍵 (GPIO 32)
  Button_Event_t menuEvent = HAL::Button_GetEvent(HAL::BUTTON_MENU_OK);
  if (menuEvent == BUTTON_EVENT_PRESS) {
    Serial.printf("Menu/OK button pressed (Current page: %d)\n", (int)currentPage);
    
    // 根據當前頁面執行不同的OK按鍵功能
    switch (currentPage) {
      case PAGE_MAINMENU:
        Serial.println("MainMenu: OK button - Confirm icon selection");
        if (pageManager->getMenuView()) {
          MenuIcon selectedIcon = pageManager->getMenuView()->getSelectedIcon();
          switch (selectedIcon) {
            case MenuIcon::TREKKING:
              Serial.println("Selected: Trekking");
              pageManager->switchToPage(PAGE_TREKKING);
              break;
            case MenuIcon::RADIO:
              Serial.println("Selected: Radio (WalkieTalkie)");
              pageManager->switchToPage(PAGE_WALKIETALKIE);
              break;
            case MenuIcon::SYSTEM:
              Serial.println("Selected: System");
              pageManager->switchToPage(PAGE_SYSTEM);
              break;
            case MenuIcon::STATUS:
              Serial.println("Selected: Status");
              pageManager->switchToPage(PAGE_STATUS);
              break;
          }
        }
        break;
        
      case PAGE_TREKKING:
        Serial.println("Trekking: OK button - Handle trekking action");
        if (pageManager->getTrekkingView()) {
          pageManager->getTrekkingView()->handleOKButton();
        }
        break;
        
      case PAGE_WALKIETALKIE:
        Serial.println("WalkieTalkie: OK button - Handle operation mode");
        if (pageManager->getWalkieTalkieView()) {
          pageManager->getWalkieTalkieView()->handleOKButton();
        }
        break;
        
      case PAGE_STATUS:
        Serial.println("Status: OK button - Toggle scroll mode");
        scrollMode = !scrollMode;
        Serial.printf("Scroll mode: %s\n", scrollMode ? "ON" : "OFF");
        break;
        
      case PAGE_SYSTEM:
        Serial.println("System: OK button - Confirm selection");
        if (pageManager->getSystemView()) {
          pageManager->getSystemView()->confirmSelection();
        }
        break;
        
      default:
        Serial.printf("OK button ignored (Page: %d)\n", (int)currentPage);
        break;
    }
  } else if (menuEvent == BUTTON_EVENT_HOLD) {
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
  Button_Event_t upEvent = HAL::Button_GetEvent(HAL::BUTTON_UP_BACK);
  if (upEvent == BUTTON_EVENT_PRESS) {
    if (currentPage == PAGE_MAINMENU) {
      // MainMenu頁面：UP按鍵選擇上一個圖標
      Serial.println("UP button pressed - MainMenu select previous icon");
      if (pageManager->getMenuView()) {
        pageManager->getMenuView()->selectPrevIcon();
      }
    } else if (scrollMode) {
      // 滾動模式：UP按鍵用於螢幕向上滾動
      Serial.println("UP button pressed - Screen scroll up");
      switch (currentPage) {
        case PAGE_TREKKING:
          if (pageManager->getTrekkingView()) {
            pageManager->getTrekkingView()->handleUpButton();
          }
          break;
        case PAGE_WALKIETALKIE:
          if (pageManager->getWalkieTalkieView()) {
            pageManager->getWalkieTalkieView()->handleUpButton();
          }
          break;
        case PAGE_STATUS:
          if (pageManager->getStatusPresenter()) {
            pageManager->getStatusPresenter()->scrollUp();
          }
          break;
        case PAGE_SYSTEM:
          if (pageManager->getSystemView()) {
            pageManager->getSystemView()->selectPrevItem();
          }
          break;
        default:
          break;
      }
    } else {
      // 非滾動模式：UP按鍵用於頁面切換 (除了 WalkieTalkie)
      if (currentPage == PAGE_WALKIETALKIE) {
        // WalkieTalkie 頁面：UP 按鍵總是用於頁面內操作
        Serial.println("UP button pressed - WalkieTalkie internal operation");
        if (pageManager->getWalkieTalkieView()) {
          pageManager->getWalkieTalkieView()->handleUpButton();
        }
      } else {
        PageID nextPage;
        switch (currentPage) {
          case PAGE_TREKKING:
            nextPage = PAGE_WALKIETALKIE;
            Serial.println("UP button pressed - Trekking -> WalkieTalkie");
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
  }
  
  // 檢查DOWN按鍵 (GPIO 34)
  Button_Event_t downEvent = HAL::Button_GetEvent(HAL::BUTTON_DOWN_FN);
  if (downEvent == BUTTON_EVENT_PRESS) {
    if (currentPage == PAGE_MAINMENU) {
      // MainMenu頁面：DOWN按鍵選擇下一個圖標
      Serial.println("DOWN button pressed - MainMenu select next icon");
      if (pageManager->getMenuView()) {
        pageManager->getMenuView()->selectNextIcon();
      }
    } else if (scrollMode) {
      // 滾動模式：DOWN按鍵用於螢幕向下滾動
      Serial.println("DOWN button pressed - Screen scroll down");
      switch (currentPage) {
        case PAGE_TREKKING:
          if (pageManager->getTrekkingView()) {
            pageManager->getTrekkingView()->handleDownButton();
          }
          break;
        case PAGE_WALKIETALKIE:
          if (pageManager->getWalkieTalkieView()) {
            pageManager->getWalkieTalkieView()->handleDownButton();
          }
          break;
        case PAGE_STATUS:
          if (pageManager->getStatusPresenter()) {
            pageManager->getStatusPresenter()->scrollDown();
          }
          break;
        case PAGE_SYSTEM:
          if (pageManager->getSystemView()) {
            pageManager->getSystemView()->selectNextItem();
          }
          break;
        default:
          break;
      }
    } else {
      // 非滾動模式：DOWN按鍵用於頁面切換 (除了 WalkieTalkie)
      if (currentPage == PAGE_WALKIETALKIE) {
        // WalkieTalkie 頁面：DOWN 按鍵總是用於頁面內操作
        Serial.println("DOWN button pressed - WalkieTalkie internal operation");
        if (pageManager->getWalkieTalkieView()) {
          pageManager->getWalkieTalkieView()->handleDownButton();
        }
      } else {
        PageID prevPage;
        switch (currentPage) {
          case PAGE_TREKKING:
            prevPage = PAGE_MAINMENU;
            Serial.println("DOWN button pressed - Trekking -> MainMenu");
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
    }
  } else if (downEvent == BUTTON_EVENT_HOLD) {
    if (currentPage == PAGE_WALKIETALKIE) {
      Serial.println("WalkieTalkie: Down button hold - Return to MainMenu");
      pageManager->switchToPage(PAGE_MAINMENU);
      scrollMode = false;
    } else {
      Serial.println("Down button hold - reserved function");
      // 長按：保留功能（可以分配給其他用途）
    }
  } else if (downEvent == BUTTON_EVENT_DOUBLE) {
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
  
  Serial.println("=== ESP32 OLED Application with HAL System ===");

  // Initialize HAL System (handles all hardware)
  HAL::HAL_Init();
  delay(1000);
  
  // DataProc initialization moved after LVGL init (requires lv_mem_alloc)

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
  
  Serial.println("=== Setup Complete - HAL System Ready ===");
}

void loop() {
  // Handle LVGL tasks
  lv_timer_handler();
  
  // Handle button events using HAL
  handleButtonEvents();
  
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
  
  // Small delay to prevent watchdog issues
  delay(5);
}