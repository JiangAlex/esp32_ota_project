#ifndef SETTINGS_MODEL_H
#define SETTINGS_MODEL_H

#include "../../MVP/BaseModel.h"

struct SettingItem {
    const char* name;
    const char* value;
    bool editable;
};

class SettingsModel : public BaseModel {
private:
    SettingItem settings[8];
    int selectedIndex;

public:
    SettingsModel();
    
    const SettingItem* getSettings() const { return settings; }
    int getSettingsCount() const { return 8; }
    int getSelectedIndex() const { return selectedIndex; }
    void setSelectedIndex(int index);
    
    // 為了與 SettingsPresenter 兼容的方法
    int getCurrentSelection() const { return selectedIndex; }
    void setCurrentSelection(int index) { setSelectedIndex(index); }
    int getSettingCount() const { return 8; }
    const SettingItem* getSettingItem(int index) const;
    
    void updateSettingValue(int index, const char* newValue);
    const SettingItem& getSelectedSetting() const;
};

#endif // SETTINGS_MODEL_H