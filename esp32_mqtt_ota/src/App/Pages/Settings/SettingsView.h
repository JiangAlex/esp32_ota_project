#ifndef SETTINGS_VIEW_H
#define SETTINGS_VIEW_H

#include "../../MVP/BaseView.h"

class SettingsView : public BaseView {
private:
    lv_obj_t* screen;
    bool created;

public:
    SettingsView();
    ~SettingsView();
    
    void create() override;
    void destroy() override;
    lv_obj_t* getScreen() const override;
    bool isCreated() const override;
};

#endif // SETTINGS_VIEW_H