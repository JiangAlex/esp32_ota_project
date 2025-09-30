#include "MenuModel.h"

MenuModel::MenuModel() : selectedIndex(0) {
    menuItems[0] = {"PM5544 Test", 0, true};
    menuItems[1] = {"Settings", 1, true};
    menuItems[2] = {"Status", 2, true};
    menuItems[3] = {"Exit", 3, true};
}

void MenuModel::setSelectedIndex(int index) {
    if (index >= 0 && index < 4) {
        selectedIndex = index;
    }
}

const MenuItem& MenuModel::getSelectedItem() const {
    return menuItems[selectedIndex];
}