/*
 * SA818 頻道管理使用範例
 * 
 * 此檔案展示如何使用新的 SA818 頻道管理功能
 * 包含 L-CH (Low Power) 和 H-CH (High Power) 兩種功率模式
 * 每種模式都有 20 個預設頻道
 */

#include "../../Common/HAL/HAL.h"
#include "../../Common/HAL/SA818_Channels.h"
#include <Arduino.h>

class SA818_Example {
public:
    // 初始化 SA818 並設定預設頻道
    static void init() {
        Serial.println("SA818 Example: Initializing...");
        
        // 初始化 SA818
        HAL::SA818_Init();
        
        // 設定預設頻道：L-CH 模式，頻道 1 (409.75 MHz)
        HAL::SA818_SetChannel(1, SA818_LOW_POWER);
        
        Serial.println("SA818 Example: Initialized with L-CH Channel 1");
        displayCurrentStatus();
    }
    
    // 顯示當前狀態
    static void displayCurrentStatus() {
        SA818_ChannelInfo_t info;
        HAL::SA818_GetChannelInfo(&info);
        
        Serial.println("=== SA818 Current Status ===");
        Serial.printf("Channel: %d\n", info.channel);
        Serial.printf("Power Mode: %s\n", info.powerModeName);
        Serial.printf("Frequency: %.4f MHz\n", info.frequency);
        Serial.printf("Range: %s\n", info.frequencyRange);
        Serial.println("============================");
    }
    
    // 切換到下一個頻道
    static void nextChannel() {
        Serial.println("SA818 Example: Switching to next channel...");
        HAL::SA818_NextChannel();
        displayCurrentStatus();
    }
    
    // 切換到上一個頻道
    static void previousChannel() {
        Serial.println("SA818 Example: Switching to previous channel...");
        HAL::SA818_PreviousChannel();
        displayCurrentStatus();
    }
    
    // 切換功率模式 (L-CH <-> H-CH)
    static void togglePowerMode() {
        Serial.println("SA818 Example: Toggling power mode...");
        HAL::SA818_TogglePowerMode();
        displayCurrentStatus();
    }
    
    // 設定特定頻道和功率模式
    static void setSpecificChannel(int channel, SA818_PowerMode powerMode) {
        Serial.printf("SA818 Example: Setting channel %d in %s mode...\n", 
                      channel, getPowerModeName(powerMode));
        
        if (HAL::SA818_SetChannel(channel, powerMode)) {
            Serial.println("SA818 Example: Channel set successfully");
            displayCurrentStatus();
        } else {
            Serial.println("SA818 Example: Failed to set channel");
        }
    }
    
    // 掃描所有 L-CH 頻道
    static void scanLowPowerChannels() {
        Serial.println("SA818 Example: Scanning all L-CH channels...");
        
        for (int ch = 1; ch <= SA818_MAX_CHANNELS; ch++) {
            float freq = getSA818Frequency(SA818_LOW_POWER, ch);
            Serial.printf("L-CH Channel %2d: %.4f MHz\n", ch, freq);
            delay(100); // 避免過快輸出
        }
    }
    
    // 掃描所有 H-CH 頻道
    static void scanHighPowerChannels() {
        Serial.println("SA818 Example: Scanning all H-CH channels...");
        
        for (int ch = 1; ch <= SA818_MAX_CHANNELS; ch++) {
            float freq = getSA818Frequency(SA818_HIGH_POWER, ch);
            Serial.printf("H-CH Channel %2d: %.4f MHz\n", ch, freq);
            delay(100); // 避免過快輸出
        }
    }
    
    // 測試所有功能
    static void runFullTest() {
        Serial.println("SA818 Example: Running full test...");
        
        // 1. 初始化
        init();
        delay(1000);
        
        // 2. 測試頻道切換
        Serial.println("\n--- Testing Channel Navigation ---");
        for (int i = 0; i < 3; i++) {
            nextChannel();
            delay(500);
        }
        
        for (int i = 0; i < 2; i++) {
            previousChannel();
            delay(500);
        }
        
        // 3. 測試功率模式切換
        Serial.println("\n--- Testing Power Mode Toggle ---");
        togglePowerMode(); // L-CH -> H-CH
        delay(500);
        togglePowerMode(); // H-CH -> L-CH
        delay(500);
        
        // 4. 測試特定頻道設定
        Serial.println("\n--- Testing Specific Channel Setting ---");
        setSpecificChannel(10, SA818_HIGH_POWER);
        delay(500);
        setSpecificChannel(5, SA818_LOW_POWER);
        delay(500);
        
        // 5. 顯示頻道列表
        Serial.println("\n--- All Available Channels ---");
        scanLowPowerChannels();
        delay(1000);
        scanHighPowerChannels();
        
        Serial.println("SA818 Example: Full test completed");
    }
};

// 在 WalkieTalkie 頁面中使用的範例
void WalkieTalkieView_ExampleUsage() {
    // 基本使用範例
    SA818_Example::init();
    
    // 模擬使用者操作
    SA818_Example::nextChannel();       // UP 按鈕
    SA818_Example::previousChannel();   // DOWN 按鈕
    SA818_Example::togglePowerMode();   // 功率切換
    
    // 設定特定頻道 (例如：應急頻道)
    SA818_Example::setSpecificChannel(1, SA818_LOW_POWER);   // L-CH Ch1: 409.75 MHz
    SA818_Example::setSpecificChannel(10, SA818_HIGH_POWER); // H-CH Ch10: 434.4375 MHz
}