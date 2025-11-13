#ifndef MENU_VIEW_H
#define MENU_VIEW_H

#include "../../MVP/BaseView.h"
#include "MenuModel.h"

// 圖標選項枚舉
enum class MenuIcon {
    TREKKING = 0,    // ⛰️ Trekking
    RADIO = 1,       // 🎙️ Radio  
    SYSTEM = 2,      // ⚙️ System
    STATUS = 3       // 📶 Status
};

class MenuView : public BaseView {
private:
    lv_obj_t* screen;
    bool created;
    MenuModel* model;
    
    // 圖標式選單組件
    lv_obj_t* statusBar;        // 頂部狀態欄
    lv_obj_t* iconContainer;    // 圖標容器 (2x2 矩陣)
    lv_obj_t* iconLabels[4];    // 圖標標籤
    lv_obj_t* iconHighlight;    // 選中高亮框
    lv_obj_t* hintArea;         // 底部提示區
    
    int selectedIcon;           // 當前選中的圖標 (0-3)

public:
    MenuView(MenuModel* m);
    ~MenuView();
    
    void create() override;
    void destroy() override;
    lv_obj_t* getScreen() const override;
    bool isCreated() const override;
    
    // 圖標選單操作
    void selectNextIcon();      // UP 按鍵：向上選擇
    void selectPrevIcon();      // DOWN 按鍵：向下選擇  
    void confirmSelection();    // OK 按鍵：確認選擇
    void updateSelection(int index);  // 更新選中項目
    void updateIconHighlight(); // 更新選中高亮
    
    // 獲取當前選中的圖標
    MenuIcon getSelectedIcon() const { return static_cast<MenuIcon>(selectedIcon); }
    
    // 時間更新相關方法
    void updateTimeAndDate();
    void updateBatteryLevel();
    void updateStatusBar(const char* batteryText, const char* timeText);
    
private:
    void createIconMenuLayout();
    void createIconButton(int index, int row, int col, const char* iconText, const char* labelText);
    
    // 時間相關私有方法  
    void getRealTime(char* timeStr, char* dateStr);
    void initRTC();
    
    // 圖標位置計算
    void getIconPosition(int index, int* x, int* y);
    lv_obj_t* timeLabel;
    lv_obj_t* dateLabel;
};

#endif // MENU_VIEW_H