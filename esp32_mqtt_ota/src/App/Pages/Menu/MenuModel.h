#ifndef MENU_MODEL_H
#define MENU_MODEL_H

#include "../../MVP/BaseModel.h"

struct MenuItem {
    const char* text;
    int id;
    bool enabled;
};

class MenuModel : public BaseModel {
private:
    MenuItem menuItems[4];
    int selectedIndex;
    
public:
    MenuModel();
    
    const MenuItem* getMenuItems() const { return menuItems; }
    int getMenuItemCount() const { return 4; }
    int getSelectedIndex() const { return selectedIndex; }
    void setSelectedIndex(int index);
    const MenuItem& getSelectedItem() const;
};

#endif // MENU_MODEL_H