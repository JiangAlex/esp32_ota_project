#ifndef MENU_VIEW_H
#define MENU_VIEW_H

#include "../../MVP/BaseView.h"
#include "MenuModel.h"

class MenuView : public BaseView {
private:
    lv_obj_t* screen;
    bool created;
    MenuModel* model;
    lv_obj_t* buttons[4];
    lv_obj_t* title;

public:
    MenuView(MenuModel* m);
    ~MenuView();
    
    void create() override;
    void destroy() override;
    lv_obj_t* getScreen() const override;
    bool isCreated() const override;
    
    void updateSelection(int index);
    
    // 時間更新相關方法
    void updateTimeAndDate();
    void updateBatteryLevel();
    
private:
    void createMainMenuLayout();
    static void button_event_cb(lv_event_t* e);
    
    // 時間相關私有方法
    void getRealTime(char* timeStr, char* dateStr);
    void initRTC();
    
    // MainMenu界面元素
    lv_obj_t* menuArea;
    lv_obj_t* batteryLabel;
    lv_obj_t* timeLabel;
    lv_obj_t* dateLabel;
};

#endif // MENU_VIEW_H