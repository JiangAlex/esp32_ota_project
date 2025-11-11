#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>

// 按鍵定義
#define BTN_MENU_OK_PIN   32  // GPIO 32 - Menu/OK 按鈕
#define BTN_UP_BACK_PIN   33  // GPIO 33 - UP/Back 按鈕
#define BTN_DOWN_FN_PIN   34  // GPIO 34 - Down/Fn 按鈕 (輸入專用，需外部上拉)

// 按鍵狀態
enum ButtonState {
    BTN_RELEASED = 0,
    BTN_PRESSED = 1,
    BTN_HOLD = 2
};

// 按鍵事件類型
enum ButtonEvent {
    BTN_EVENT_NONE = 0,
    BTN_EVENT_PRESS,    // 短按
    BTN_EVENT_RELEASE,  // 釋放
    BTN_EVENT_HOLD,     // 長按 (>2000ms)
    BTN_EVENT_DOUBLE    // 雙擊 (<300ms間隔)
};

struct ButtonInfo {
    uint8_t pin;
    ButtonState state;
    ButtonState lastState;
    unsigned long pressTime;
    unsigned long releaseTime;
    unsigned long lastPressTime;
    bool isHolding;
    bool doubleClickFlag;
};

class ButtonManager {
private:
    ButtonInfo buttons[3];
    static ButtonManager* instance;
    
    // 按鍵去抖動參數
    static const unsigned long DEBOUNCE_DELAY = 50;    // 50ms去抖動
    static const unsigned long HOLD_TIME = 2000;       // 2000ms長按 (2秒)
    static const unsigned long DOUBLE_CLICK_TIME = 300; // 300ms雙擊間隔
    
public:
    static ButtonManager* getInstance();
    
    void init();
    void update();
    
    // 獲取按鍵事件
    ButtonEvent getButtonEvent(uint8_t buttonIndex);
    
    // 檢查按鍵狀態
    bool isPressed(uint8_t buttonIndex);
    bool isReleased(uint8_t buttonIndex);
    bool isHold(uint8_t buttonIndex);
    
    // 按鍵索引
    enum ButtonIndex {
        BTN_MENU_OK = 0,   // GPIO 32
        BTN_UP_BACK = 1,   // GPIO 33
        BTN_DOWN_FN = 2    // GPIO 34
    };
    
private:
    ButtonManager();
    void updateButton(uint8_t index);
};

#endif // BUTTON_MANAGER_H