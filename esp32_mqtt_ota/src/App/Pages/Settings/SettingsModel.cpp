#include "SettingsModel.h"

SettingsModel::SettingsModel() : selectedIndex(0) {
    settings[0] = {"WiFi Settings", "Disconnected", true};
    settings[1] = {"Display Settings", "Brightness: 80%", true};
    settings[2] = {"System Settings", "Auto Sleep: 5min", true};
    settings[3] = {"Button Config", "Standard", true};
    settings[4] = {"Screen Timeout", "30 seconds", true};
    settings[5] = {"Brightness", "80%", true};
    settings[6] = {"Language", "English", true};
    settings[7] = {"Factory Reset", "Reset All", true};
}

void SettingsModel::setSelectedIndex(int index) {
    if (index >= 0 && index < 8) {
        selectedIndex = index;
    }
}

void SettingsModel::updateSettingValue(int index, const char* newValue) {
    if (index >= 0 && index < 8 && settings[index].editable) {
        settings[index].value = newValue;
    }
}

const SettingItem& SettingsModel::getSelectedSetting() const {
    return settings[selectedIndex];
}

const SettingItem* SettingsModel::getSettingItem(int index) const {
    if (index >= 0 && index < 8) {
        return &settings[index];
    }
    return nullptr;
}