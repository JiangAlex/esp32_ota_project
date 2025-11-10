#ifndef SETTINGS_PRESENTER_H
#define SETTINGS_PRESENTER_H

#include "../../MVP/BasePresenter.h"
#include "SettingsView.h"
#include "SettingsModel.h"

class SettingsPresenter : public BasePresenter<SettingsView> {
private:
    SettingsModel* model;

public:
    SettingsPresenter(SettingsView* view, SettingsModel* model);
    ~SettingsPresenter();
    
    void onCreate() override;
    void onDestroy() override;
    void onShow() override;
    void onHide() override;
    
    // 業務邏輯方法
    void updateSettingsDisplay();
    void scrollUp();
    void scrollDown();
    void selectSetting(int index);
    void executeSelectedSetting();
};

#endif // SETTINGS_PRESENTER_H