#include "HAL.h"
#include <LovyanGFX.hpp>
#include "LGFX_ChappieCore.hpp"

static LGFX* display_instance = nullptr;

void HAL::Display_Init()
{
    if (!display_instance) {
        display_instance = new LGFX();
    }
    
    Serial.println("正在初始化 OLED 顯示器...");
    Serial.println("I2C 配置: SDA=21, SCL=22, Address=0x3C");
    
    bool initResult = display_instance->init();
    if (initResult) {
        Serial.println("✅ LGFX 初始化成功");
    } else {
        Serial.println("❌ LGFX 初始化失敗 - 請檢查硬體連接");
    }
    
    display_instance->setRotation(0); // OLED通常使用0度旋轉
    display_instance->fillScreen(TFT_BLACK);
    
    // 測試顯示器是否工作
    Display_Test();
    
    Serial.printf("顯示器尺寸: %dx%d\n", display_instance->width(), display_instance->height());
    Serial.println("SSD1306 128x64 OLED Display initialized");
}

void HAL::Display_Test()
{
    if (!display_instance) return;
    
    display_instance->setTextColor(TFT_WHITE);
    display_instance->setTextSize(1);
    display_instance->setCursor(0, 0);
    display_instance->println("ESP32 OLED Test");
    display_instance->println("SSD1306 128x64");
    
    // 在主資訊區置中顯示 "SoftSnail" (16px字體大小)
    display_instance->setTextSize(2); // 16px字體 (8x2=16)
    
    // 計算置中位置 (128px寬度的螢幕)
    const char* text = "SoftSnail";
    int textWidth = strlen(text) * 12; // 大約每個字元12px寬度 (size=2時)
    int x = (128 - textWidth) / 2;
    int y = 32; // 螢幕中央位置
    
    display_instance->setCursor(x, y);
    display_instance->println(text);
}

void HAL::Display_Clear()
{
    if (!display_instance) return;
    display_instance->fillScreen(TFT_BLACK);
}

void HAL::Display_SetBrightness(uint8_t brightness)
{
    if (!display_instance) return;
    
    // OLED brightness control using SSD1306 commands
    display_instance->setBrightness(brightness);
}

void HAL::Display_Sleep()
{
    if (!display_instance) return;
    
    // Put OLED into sleep mode (turns off display completely)
    display_instance->sleep();
    Serial.println("HAL: OLED put into sleep mode (power OFF)");
}

void HAL::Display_Wakeup()
{
    if (!display_instance) return;
    
    // Wake up OLED from sleep mode
    display_instance->wakeup();
    Serial.println("HAL: OLED wakeup from sleep mode (power ON)");
}

void HAL::Display_GetInfo(::Display_Info_t *info)
{
    if (!info || !display_instance) return;
    
    info->width = display_instance->width();
    info->height = display_instance->height();
    info->brightness = 255; // Default for OLED
    info->isOn = true;
}

void HAL::Display_DumpCrashInfo(const char *info)
{
    if (!display_instance || !info) return;
    
    display_instance->fillScreen(TFT_BLACK);
    display_instance->setTextColor(TFT_WHITE);
    display_instance->setTextSize(1);
    display_instance->setCursor(0, 0);
    display_instance->println("CRASH INFO:");
    display_instance->println(info);
}

void HAL::Display_SetAddrWindow(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
    if (!display_instance) return;
    
    uint32_t w = (x1 - x0 + 1);
    uint32_t h = (y1 - y0 + 1);
    display_instance->setAddrWindow(x0, y0, w, h);
}

void HAL::Display_SendPixels(const uint16_t *pixels, uint32_t len)
{
    if (!display_instance || !pixels) return;
    
    display_instance->startWrite();
    display_instance->pushPixels((uint16_t*)pixels, len);
    display_instance->endWrite();
}

// Global callback function pointer
static void (*display_finish_callback)(void) = nullptr;

void HAL::Display_SetSendFinishCallback(void (*func)(void))
{
    display_finish_callback = func;
}

// Get display instance for LVGL integration
LGFX* HAL_GetDisplayInstance()
{
    return display_instance;
}