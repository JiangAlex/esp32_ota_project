#ifndef SETTINGS_VIEW_H
#define SETTINGS_VIEW_H

#include "../../MVP/BaseView.h"

class SettingsView : public BaseView {
private:
    lv_obj_t* screen;
    lv_obj_t* contentLabel;
    bool created;

public:
    SettingsView();
    ~SettingsView();
    
    void create() override;
    void destroy() override;
    lv_obj_t* getScreen() const override;
    bool isCreated() const override;
    void scrollUp();
    void scrollDown();
};

#endif // SETTINGS_VIEW_H