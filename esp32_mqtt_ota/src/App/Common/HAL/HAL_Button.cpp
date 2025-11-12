#include "HAL.h"

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