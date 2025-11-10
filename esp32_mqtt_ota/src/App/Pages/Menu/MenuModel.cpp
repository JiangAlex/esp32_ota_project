#include "MenuModel.h"

MenuModel::MenuModel() : selectedIndex(0) {
    menuItems[0] = {"Settings", 0, true};
    menuItems[1] = {"Status", 1, true};
    menuItems[2] = {"", 2, false}; // 未使用的項目
    menuItems[3] = {"", 3, false}; // 未使用的項目
}

void MenuModel::setSelectedIndex(int index) {
    if (index >= 0 && index < 2) {
        selectedIndex = index;
    }
}

const MenuItem& MenuModel::getSelectedItem() const {
    return menuItems[selectedIndex];
}