#pragma once
#include "../../MVP/BaseView.h"
#include <lvgl.h>

// 無線電狀態結構
struct RadioStatus {
    int channel;            // 頻道號碼
    float frequency;        // 頻率 (MHz)
    float ctcss;           // CTCSS 靜噪頻率 (Hz)
    const char* powerLevel; // 功率等級 ("HIGH", "MID", "LOW")
    int rssi;              // 訊號強度 (dBm)
    const char* rxTxMode;  // 當前模式 ("RX", "TX", "IDLE")
    int squelchLevel;      // 靜噪等級 (1-8)
    int volumeLevel;       // 音量等級 (1-8)
    
    // 初始化預設值
    RadioStatus() : channel(15), frequency(462.5625f), ctcss(88.5f), 
                    powerLevel("HIGH"), rssi(-85), rxTxMode("RX"),
                    squelchLevel(3), volumeLevel(6) {}
};

// 操作模式枚舉
enum class OperationMode {
    NAVIGATION,     // 導航模式：箭頭選擇項目，提示區顯示 [OK]
    VALUE_EDIT      // 數值編輯模式：UP/DOWN修改數值，提示區顯示 [BACK]
};

class WalkieTalkieView : public BaseView {
private:
    lv_obj_t* screen;
    lv_obj_t* statusBar;        // 頂部狀態欄
    lv_obj_t* contentArea;      // 內容區域
    // 變更為支援列表模式 (6 項目，可視區塊滾動)
    static const int ITEM_COUNT = 6;
    static const int VISIBLE_COUNT = 4; // 一次可見行數 (在 38px 區域中顯示)
    lv_obj_t* radioLabels[ITEM_COUNT];   // 無線電資訊標籤
    lv_obj_t* arrowLabel;                 // 選取箭頭 '>'
    lv_obj_t* hintBar;                    // 底部提示區

    bool created;
    RadioStatus radioStatus;    // 無線電狀態資料
    int selectionIndex;         // 當前選中項目 (0..ITEM_COUNT-1)
    int scrollOffset;           // 滾動偏移 (第一個可見項目的索引)
    OperationMode currentMode;  // 當前操作模式

public:
    WalkieTalkieView();
    ~WalkieTalkieView();
    
    void create() override;
    void destroy() override;
    lv_obj_t* getScreen() const override;
    bool isCreated() const override;
    
    // 無線電控制功能
    void updateRadioStatus(const RadioStatus& status);
    void adjustSquelch(int delta);      // 調整靜噪等級
    void adjustVolume(int delta);       // 調整音量等級
    void changeChannel(int delta);      // 切換頻道
    void togglePower();                 // 切換功率等級
    
    // 按鈕處理
    void handleOKButton();      // OK: 切換操作模式
    void handleUpButton();      // UP: 根據模式執行不同動作
    void handleDownButton();    // DOWN: 根據模式執行不同動作
    
    // 操作模式管理
    void setOperationMode(OperationMode mode);
    OperationMode getOperationMode() const { return currentMode; }
    void updateHintText();      // 更新提示文字
    
    // 狀態欄更新
    void updateStatusBar(const char* batteryText, const char* timeText);
    
    // 獲取當前狀態
    const RadioStatus& getRadioStatus() const { return radioStatus; }

private:
    // 內部方法
    void updateDisplay();
    void formatFrequency(float freq, char* buffer, size_t bufferSize);
    // 更新選取顯示（箭頭位置與滾動）
    void updateSelectionDisplay();
    // 調整選中項目的數值
    void adjustSelectedValue(int delta);
    // 從 HAL 更新無線電狀態
    void updateFromHAL();
};