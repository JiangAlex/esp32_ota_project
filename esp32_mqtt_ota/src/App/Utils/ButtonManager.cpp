#include "ButtonManager.h"

ButtonManager* ButtonManager::instance = nullptr;

ButtonManager* ButtonManager::getInstance() {
    if (instance == nullptr) {
        instance = new ButtonManager();
    }
    return instance;
}

ButtonManager::ButtonManager() {
    // 初始化按鍵信息
    buttons[BTN_MENU_OK] = {BTN_MENU_OK_PIN, BTN_RELEASED, BTN_RELEASED, 0, 0, 0, false, false};
    buttons[BTN_UP_BACK] = {BTN_UP_BACK_PIN, BTN_RELEASED, BTN_RELEASED, 0, 0, 0, false, false};
    buttons[BTN_DOWN_FN] = {BTN_DOWN_FN_PIN, BTN_RELEASED, BTN_RELEASED, 0, 0, 0, false, false};
}

void ButtonManager::init() {
    // 配置GPIO為輸入模式
    pinMode(BTN_MENU_OK_PIN, INPUT_PULLUP);   // GPIO 32 - Menu/OK (內部上拉)
    pinMode(BTN_UP_BACK_PIN, INPUT_PULLUP);   // GPIO 33 - UP/Back (內部上拉)
    pinMode(BTN_DOWN_FN_PIN, INPUT);          // GPIO 34 - Down/Fn (輸入專用，需外部上拉)
    
    Serial.println("ButtonManager initialized");
    Serial.println("GPIO 32: Menu/OK (內部上拉)");
    Serial.println("GPIO 33: UP/Back (內部上拉)"); 
    Serial.println("GPIO 34: Down/Fn (輸入專用 - 需要外部10kΩ上拉電阻到3.3V)");
    Serial.println("接線: 按鍵一端接GPIO，另一端接GND，GPIO 34需額外接10kΩ電阻到3.3V");
}

void ButtonManager::update() {
    for (int i = 0; i < 3; i++) {
        updateButton(i);
    }
}

void ButtonManager::updateButton(uint8_t index) {
    ButtonInfo& btn = buttons[index];
    unsigned long currentTime = millis();
    
    // 特殊處理GPIO 34 - 如果沒有外部上拉電阻，暫時跳過
    if (btn.pin == 34) {
        static bool gpio34_warning_shown = false;
        int gpio34_raw = digitalRead(34);
        
        // 如果GPIO 34持續為LOW超過1秒，可能是浮空或缺少上拉電阻
        static unsigned long gpio34_low_start = 0;
        if (gpio34_raw == 0) {
            if (gpio34_low_start == 0) {
                gpio34_low_start = currentTime;
            } else if (currentTime - gpio34_low_start > 1000 && !gpio34_warning_shown) {
                Serial.println("ERROR: GPIO 34持續為LOW - 請檢查是否有外部10kΩ上拉電阻");
                Serial.println("暫時停用GPIO 34按鍵功能");
                gpio34_warning_shown = true;
            }
        } else {
            gpio34_low_start = 0;  // 重置
        }
        
        // 如果GPIO 34有問題，跳過處理
        if (gpio34_warning_shown) {
            return;
        }
    }
    
    // 讀取當前按鍵狀態 (LOW = 按下，因為使用上拉電阻)
    bool currentPressed = digitalRead(btn.pin) == LOW;
    
    // 調試輸出 - 每5秒輸出一次GPIO狀態
    static unsigned long lastDebugTime = 0;
    if (currentTime - lastDebugTime > 5000) {
        int gpio32_state = digitalRead(32);
        int gpio33_state = digitalRead(33);
        int gpio34_state = digitalRead(34);
        Serial.printf("GPIO狀態 - 32:%d, 33:%d, 34:%d\n", gpio32_state, gpio33_state, gpio34_state);
        
        // 特別檢查GPIO 34是否浮空
        if (gpio34_state == 0) {
            Serial.println("WARNING: GPIO 34讀取為LOW - 可能缺少外部上拉電阻或按鍵卡住");
        }
        lastDebugTime = currentTime;
    }
    
    // 去抖動處理
    if (currentPressed != (btn.state == BTN_PRESSED)) {
        if (currentTime - btn.releaseTime > DEBOUNCE_DELAY) {
            btn.lastState = btn.state;
            
            if (currentPressed) {
                // 按鍵被按下
                btn.state = BTN_PRESSED;
                btn.pressTime = currentTime;
                btn.isHolding = false;
                
                // 檢查雙擊
                if (currentTime - btn.lastPressTime < DOUBLE_CLICK_TIME) {
                    btn.doubleClickFlag = true;
                }
                btn.lastPressTime = currentTime;
                
                Serial.printf("Button %d pressed\n", index);
            } else {
                // 按鍵被釋放
                btn.state = BTN_RELEASED;
                btn.releaseTime = currentTime;
                btn.isHolding = false;
                
                Serial.printf("Button %d released\n", index);
            }
        }
    }
    
    // 檢查長按
    if (btn.state == BTN_PRESSED && !btn.isHolding) {
        if (currentTime - btn.pressTime > HOLD_TIME) {
            btn.isHolding = true;
            Serial.printf("Button %d hold\n", index);
        }
    }
}

ButtonEvent ButtonManager::getButtonEvent(uint8_t buttonIndex) {
    if (buttonIndex >= 3) return BTN_EVENT_NONE;
    
    ButtonInfo& btn = buttons[buttonIndex];
    ButtonEvent event = BTN_EVENT_NONE;
    static bool holdEventTriggered[3] = {false, false, false}; // 防止長按重複觸發
    
    // 檢查事件類型
    if (btn.state == BTN_PRESSED && btn.lastState == BTN_RELEASED) {
        if (btn.doubleClickFlag) {
            event = BTN_EVENT_DOUBLE;
            btn.doubleClickFlag = false; // 清除雙擊標誌
        } else {
            event = BTN_EVENT_PRESS;
        }
        holdEventTriggered[buttonIndex] = false; // 重置長按標誌
    } else if (btn.state == BTN_RELEASED && btn.lastState == BTN_PRESSED) {
        event = BTN_EVENT_RELEASE;
        holdEventTriggered[buttonIndex] = false; // 重置長按標誌
    } else if (btn.isHolding && !holdEventTriggered[buttonIndex]) {
        event = BTN_EVENT_HOLD;
        holdEventTriggered[buttonIndex] = true; // 標記長按事件已觸發
    }
    
    // 清除lastState以避免重複事件
    if (event == BTN_EVENT_PRESS || event == BTN_EVENT_RELEASE) {
        btn.lastState = btn.state;
    }
    
    return event;
}

bool ButtonManager::isPressed(uint8_t buttonIndex) {
    if (buttonIndex >= 3) return false;
    return buttons[buttonIndex].state == BTN_PRESSED;
}

bool ButtonManager::isReleased(uint8_t buttonIndex) {
    if (buttonIndex >= 3) return false;
    return buttons[buttonIndex].state == BTN_RELEASED;
}

bool ButtonManager::isHold(uint8_t buttonIndex) {
    if (buttonIndex >= 3) return false;
    return buttons[buttonIndex].isHolding;
}