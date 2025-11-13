#pragma once
#include "../../MVP/BaseView.h"
#include <lvgl.h>

// 系統設定項目枚舉
enum class SystemItem {
    BRIGHTNESS = 0,    // 顯示亮度
    TIME_SETTING = 1,  // 設定時間
    BATTERY_CAL = 2    // 電池校準
};

// 系統設定數據結構
struct SystemSettings {
    int brightness;      // 亮度等級 (1-10)
    bool timeFormat24h;  // 24小時格式
    int batteryLevel;    // 電池校準百分比
    
    SystemSettings() : brightness(8), timeFormat24h(true), batteryLevel(85) {}
};

class SystemView : public BaseView {
private:
    lv_obj_t* screen;
    lv_obj_t* statusBar;      // 頂部狀態欄
    lv_obj_t* contentArea;    // 內容區域
    lv_obj_t* menuItems[3];   // 選單項目標籤
    lv_obj_t* arrowIndicator; // 箭頭指示器
    lv_obj_t* hintLabel;      // 底部提示標籤
    
    bool created;
    int selectedItem;         // 當前選中項目 (0-2)
    SystemSettings settings;  // 系統設定數據

public:
    SystemView();
    ~SystemView();
    
    void create() override;
    void destroy() override;
    lv_obj_t* getScreen() const override;
    bool isCreated() const override;
    
    // 列表導航功能
    void selectPrevItem();    // UP 按鍵：選擇上一項
    void selectNextItem();    // DOWN 按鍵：選擇下一項
    void confirmSelection();  // OK 按鍵：進入選中項目
    void updateDisplay();     // 更新顯示內容
    void updateArrowPosition(); // 更新箭頭位置
    
    // 獲取當前選中項目
    SystemItem getSelectedItem() const { return static_cast<SystemItem>(selectedItem); }
    
    // 設定值調整
    void adjustBrightness(bool increase);
    void toggleTimeFormat();
    void calibrateBattery();
    
    // 狀態欄更新
    void updateStatusBar(const char* batteryText, const char* timeText);
    
private:
    void createSystemMenuLayout();
    void createMenuItem(int index, const char* itemText);
    const char* getBrightnessText() const;
    const char* getTimeFormatText() const;
    const char* getBatteryCalText() const;
};