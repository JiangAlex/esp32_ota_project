#ifndef MENU_PRESENTER_H
#define MENU_PRESENTER_H

#include "../../MVP/BasePresenter.h"
#include "MenuView.h"
#include "MenuModel.h"

class MenuPresenter : public BasePresenter<MenuView> {
private:
    MenuModel* model;

public:
    MenuPresenter(MenuView* view, MenuModel* model);
    ~MenuPresenter();
    
    void onCreate() override;
    void onDestroy() override;
    void onShow() override;
    void onHide() override;
    
    void selectMenuItem(int index);
    void executeSelectedItem();
};

#endif // MENU_PRESENTER_H