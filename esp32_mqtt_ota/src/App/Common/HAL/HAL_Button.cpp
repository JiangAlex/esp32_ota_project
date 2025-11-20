#include "App/Utils/PageManager/PageManager.h"
#include "App/Pages/Menu/MenuPresenter.h"
#include "App/Pages/Trekking/TrekkingView.h"
#include "App/Pages/WalkieTalkie/WalkieTalkieView.h"
#include "App/Pages/Status/StatusPresenter.h"
#include "App/Pages/System/SystemView.h"

extern PageManager* pageManager;
#include "HAL.h"

static bool scrollMode = false; // 滾動模式狀態

static ::Button_Info_t buttons[3];
static bool button_initialized = false;

void HAL::Button_Init()
{
    Serial.println("Initializing buttons...");
    
    // 初始化按鈕配置
    buttons[BUTTON_MENU_OK].pin = BTN_MENU_OK_PIN;
    buttons[BUTTON_UP_BACK].pin = BTN_UP_BACK_PIN;
    buttons[BUTTON_DOWN_FN].pin = BTN_DOWN_FN_PIN;
    
    // 設置GPIO模式
    pinMode(BTN_MENU_OK_PIN, INPUT_PULLUP);
    pinMode(BTN_UP_BACK_PIN, INPUT_PULLUP);
    pinMode(BTN_DOWN_FN_PIN, INPUT_PULLUP);  // GPIO 34需要外部上拉電阻
    
    // 初始化按鈕狀態
    for (int i = 0; i < 3; i++) {
        buttons[i].event = BUTTON_EVENT_NONE;
        buttons[i].isPressed = false;
        buttons[i].pressTime = 0;
    }
    
    button_initialized = true;
    Serial.printf("Buttons initialized: GPIO %d, %d, %d\n", 
                  BTN_MENU_OK_PIN, BTN_UP_BACK_PIN, BTN_DOWN_FN_PIN);
}

void HAL::Button_Update()
{
    if (!button_initialized) return;
    
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate < BTN_DEBOUNCE_DELAY) return;
    lastUpdate = millis();
    
    for (int i = 0; i < 3; i++) {
        bool currentState = !digitalRead(buttons[i].pin); // 反邏輯，按下為LOW
        bool wasPressed = buttons[i].isPressed;
        
        // 重置事件
        buttons[i].event = BUTTON_EVENT_NONE;
        
        if (currentState && !wasPressed) {
            // 按鈕剛被按下
            buttons[i].isPressed = true;
            buttons[i].pressTime = millis();
            buttons[i].event = BUTTON_EVENT_PRESS;
            
        } else if (!currentState && wasPressed) {
            // 按鈕剛被釋放
            buttons[i].isPressed = false;
            unsigned long pressDuration = millis() - buttons[i].pressTime;
            
            if (pressDuration >= BTN_HOLD_TIME) {
                buttons[i].event = BUTTON_EVENT_HOLD;
            } else {
                buttons[i].event = BUTTON_EVENT_RELEASE;
            }
        }
    }
}

::Button_Event_t HAL::Button_GetEvent(uint8_t buttonIndex)
{
    if (buttonIndex >= 3 || !button_initialized) {
        return BUTTON_EVENT_NONE;
    }
    
    Button_Event_t event = buttons[buttonIndex].event;
    buttons[buttonIndex].event = BUTTON_EVENT_NONE; // 清除事件
    return event;
}

bool HAL::Button_IsPressed(uint8_t buttonIndex)
{
    if (buttonIndex >= 3 || !button_initialized) {
        return false;
    }
    
    return buttons[buttonIndex].isPressed;
}

bool HAL::Button_IsHold(uint8_t buttonIndex)
{
    if (buttonIndex >= 3 || !button_initialized) {
        return false;
    }
    
    if (!buttons[buttonIndex].isPressed) return false;
    
    unsigned long pressDuration = millis() - buttons[buttonIndex].pressTime;
    return pressDuration >= BTN_HOLD_TIME;
}

void HAL::Button_GetInfo(uint8_t buttonIndex, ::Button_Info_t *info)
{
    if (buttonIndex >= 3 || !info || !button_initialized) {
        return;
    }
    
    *info = buttons[buttonIndex];
}

// 按鍵事件處理函數 (使用HAL系統)
void HAL::Button_HandleEvents() {
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
        Serial.println("MainMenu: OK button - toggle/confirm selection via presenter");
        if (pageManager && pageManager->getMenuPresenter()) {
          pageManager->getMenuPresenter()->onOkPressed();
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
      // MainMenu頁面：UP按鍵由 presenter 處理（可能進入選單模式或移動選擇）
      Serial.println("UP button pressed - MainMenu presenter handleUpButton");
      if (pageManager && pageManager->getMenuPresenter()) {
        pageManager->getMenuPresenter()->handleUpButton();
        pageManager->getMenuPresenter()->onUserActivity();
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
      // 非滾動模式：UP按鍵只用於頁面內操作，不切換頁面
      switch (currentPage) {
        case PAGE_TREKKING:
          Serial.println("UP button pressed - Trekking internal operation");
          if (pageManager->getTrekkingView()) {
            pageManager->getTrekkingView()->handleUpButton();
          }
          break;
        case PAGE_WALKIETALKIE:
          Serial.println("UP button pressed - WalkieTalkie internal operation");
          if (pageManager->getWalkieTalkieView()) {
            pageManager->getWalkieTalkieView()->handleUpButton();
          }
          break;
        case PAGE_STATUS:
          Serial.println("UP button pressed - Status internal operation");
          if (pageManager->getStatusPresenter()) {
            pageManager->getStatusPresenter()->scrollUp();
          }
          break;
        case PAGE_SYSTEM:
          Serial.println("UP button pressed - System internal operation");
          if (pageManager->getSystemView()) {
            pageManager->getSystemView()->selectPrevItem();
          }
          break;
        default:
          break;
      }
    }
  }
  
  // 檢查DOWN按鍵 (GPIO 34)
  Button_Event_t downEvent = HAL::Button_GetEvent(HAL::BUTTON_DOWN_FN);
  if (downEvent == BUTTON_EVENT_PRESS) {
    if (currentPage == PAGE_MAINMENU) {
      // MainMenu頁面：DOWN按鍵由 presenter 處理（可能進入選單模式或移動選擇）
      Serial.println("DOWN button pressed - MainMenu presenter handleDownButton");
      if (pageManager && pageManager->getMenuPresenter()) {
        pageManager->getMenuPresenter()->handleDownButton();
        pageManager->getMenuPresenter()->onUserActivity();
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
      // 非滾動模式：DOWN按鍵只用於頁面內操作，不切換頁面
      switch (currentPage) {
        case PAGE_TREKKING:
          Serial.println("DOWN button pressed - Trekking internal operation");
          if (pageManager->getTrekkingView()) {
            pageManager->getTrekkingView()->handleDownButton();
          }
          break;
        case PAGE_WALKIETALKIE:
          Serial.println("DOWN button pressed - WalkieTalkie internal operation");
          if (pageManager->getWalkieTalkieView()) {
            pageManager->getWalkieTalkieView()->handleDownButton();
          }
          break;
        case PAGE_STATUS:
          Serial.println("DOWN button pressed - Status internal operation");
          if (pageManager->getStatusPresenter()) {
            pageManager->getStatusPresenter()->scrollDown();
          }
          break;
        case PAGE_SYSTEM:
          Serial.println("DOWN button pressed - System internal operation");
          if (pageManager->getSystemView()) {
            pageManager->getSystemView()->selectNextItem();
          }
          break;
        default:
          break;
      }
    }
  } else if (downEvent == BUTTON_EVENT_HOLD) {
    Serial.println("Down button hold - reserved function");
    // 長按：保留功能（可以分配給其他用途）
  } else if (downEvent == BUTTON_EVENT_DOUBLE) {
    Serial.println("Down button double click - function mode");
    // 雙擊：特殊功能模式 - 可以在這裡添加特定功能
    // 注意：不執行清屏操作，避免螢幕消失
    Serial.println("Function mode activated (placeholder for future features)");
  }
}
