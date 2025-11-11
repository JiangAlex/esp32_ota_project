#pragma once
#include "../../MVP/BaseView.h"
#include <lvgl.h>

class SystemView : public BaseView {
private:
    lv_obj_t* screen;
    lv_obj_t* contentLabel;
    lv_obj_t* statusBar;  // 頂部狀態欄
    bool created;
    int scrollOffset;

public:
    SystemView();
    ~SystemView();
    
    void create() override;
    void destroy() override;
    lv_obj_t* getScreen() const override;
    bool isCreated() const override;
    
    // 滾動功能
    void scrollUp();
    void scrollDown();
    void updateStatusBar(const char* batteryText, const char* timeText);
};