#pragma once
#include "../../MVP/BaseView.h"
#include <lvgl.h>

// Trekking 狀態枚舉
enum class TrekkingState {
    START,      // 準備/啟動模式
    RUNNING,    // 執行/記錄模式
    PAUSED,     // 暫停模式
    END         // 結束模式
};

// Trekking 數據結構
struct TrekkingData {
    // 實時環境數據
    float temperature;      // 溫度 (°C)
    float altitude;         // 海拔 (m)
    float pressure;         // 氣壓 (hPa)
    
    // 累計數據
    unsigned long elapsedTime;  // 累計時間 (秒)
    float distance;             // 累計距離 (km)
    float ascent;               // 累計爬升 (m)
    int stepCount;              // 步數
    
    // 初始化
    TrekkingData() : temperature(25.5), altitude(520), pressure(1012), 
                     elapsedTime(0), distance(0), ascent(0), stepCount(0) {}
};

class TrekkingView : public BaseView {
private:
    lv_obj_t* screen;
    lv_obj_t* statusBar;
    lv_obj_t* contentArea;
    lv_obj_t* dataLabels[4];    // 顯示標籤（只使用dataLabels[0]）
    
    bool created;
    TrekkingState currentState;
    TrekkingData trekkingData;
    unsigned long startTime;    // 開始時間戳
    unsigned long downKeyPressTime; // DOWN鍵按下時間戳
    bool downKeyPressed;        // DOWN鍵是否正在按下

public:
    TrekkingView();
    ~TrekkingView();
    
    void create() override;
    void destroy() override;
    lv_obj_t* getScreen() const override;
    bool isCreated() const override;
    
    // 狀態管理
    void setState(TrekkingState state);
    TrekkingState getState() const { return currentState; }
    
    // 數據更新
    void updateEnvironmentData(float temp, float alt, float press);
    void updateCumulativeData(unsigned long elapsed, float dist, float asc, int steps);
    void updateDisplay();
    
    // 按鈕處理
    void handleOKButton();
    void handleUpButton();
    void handleDownButton();
    void handleDownButtonPress();   // DOWN鍵按下
    void handleDownButtonRelease(); // DOWN鍵釋放
    void checkDownKeyLongPress();   // 檢查DOWN鍵長按
    
    // 滾動方法（參考Status頁面）
    void scrollUp();            // UP 按鍵：向上滾動
    void scrollDown();          // DOWN 按鍵：向下滾動
    
    // 狀態欄更新
    void updateStatusBar(const char* batteryText, const char* timeText);
    
private:
    // 內部方法
    void createStartLayout();
    void createRunningLayout();
    void createEndLayout();
    void formatTime(unsigned long seconds, char* buffer, size_t bufferSize);
};