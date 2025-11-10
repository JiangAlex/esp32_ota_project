#ifndef OLED_LAYOUT_H
#define OLED_LAYOUT_H

#include <lvgl.h>

// OLED 128x64 布局常數
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_TOP_HEIGHT 18    // 上半部標題區域（單色 OLED）
#define OLED_BOTTOM_HEIGHT 46   // 下半部內容區域（單色 OLED）

// 保持向下兼容的宏定義
#define OLED_TOP_YELLOW_HEIGHT OLED_TOP_HEIGHT
#define OLED_BOTTOM_BLUE_HEIGHT OLED_BOTTOM_HEIGHT

class OLEDLayout {
public:
    // 創建通用的頂部標題欄（單色 OLED 上區域 18px）
    static lv_obj_t* createTopBar(lv_obj_t* parent, const char* title);
    
    // 創建底部內容區域（單色 OLED 下區域 46px）
    static lv_obj_t* createContentArea(lv_obj_t* parent);
    
    // 更新頂部標題文字
    static void updateTopBarTitle(lv_obj_t* topBar, const char* newTitle);
    
    // 設置OLED優化的字體和樣式
    static void applyOLEDStyle(lv_obj_t* obj);
};

#endif // OLED_LAYOUT_H