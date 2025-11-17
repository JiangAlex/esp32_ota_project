#ifndef MENU_PRESENTER_H
#define MENU_PRESENTER_H

#include "../../MVP/BasePresenter.h"
#include "MenuView.h"
#include "MenuModel.h"

class MenuPresenter : public BasePresenter<MenuView> {
private:
    MenuModel* model;
    // selection / inactivity timer
    bool selectionActive;
    unsigned long lastActivityMs;
    // OLED auto-off functionality
    bool displayOn;
    unsigned long lastDisplayActivityMs;

public:
    MenuPresenter(MenuView* view, MenuModel* model);
    ~MenuPresenter();
    
    void onCreate() override;
    void onDestroy() override;
    void onShow() override;
    void onHide() override;
    
    void selectMenuItem(int index);
    void executeSelectedItem();
    void moveUp();
    void moveDown();
    int getCurrentSelection() const;
    
    // MainMenu specific controls
    void onOkPressed();               // OK button behavior (enter menu / confirm)
    void onUserActivity();            // reset inactivity timer
    bool isSelectionActive() const;
    
    // 按鈕處理方法
    void handleUpButton();
    void handleDownButton();
    
    // 時間更新方法
    void updateDisplay();
    
    // OLED 控制方法
    void turnOffDisplay();
    void turnOnDisplay();
    bool isDisplayOn() const;
};

#endif // MENU_PRESENTER_H