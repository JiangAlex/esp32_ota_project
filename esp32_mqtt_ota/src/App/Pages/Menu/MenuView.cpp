#include "MenuView.h"
#include <Arduino.h>

MenuView::MenuView(MenuModel* m) : model(m), screen(nullptr), created(false), title(nullptr) {
    for(int i = 0; i < 4; i++) {
        buttons[i] = nullptr;
    }
}

MenuView::~MenuView() {
    destroy();
}

void MenuView::create() {
    if (created) return;
    
    screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x002244), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    
    title = lv_label_create(screen);
    lv_label_set_text(title, "Main Menu");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(title, 10, 10);
    
    // Create menu buttons
    const MenuItem* items = model->getMenuItems();
    for(int i = 0; i < model->getMenuItemCount(); i++) {
        buttons[i] = lv_btn_create(screen);
        lv_obj_set_size(buttons[i], 200, 40);
        lv_obj_set_pos(buttons[i], 60, 50 + i * 50);
        lv_obj_add_event_cb(buttons[i], button_event_cb, LV_EVENT_CLICKED, (void*)(intptr_t)i);
        
        lv_obj_t *label = lv_label_create(buttons[i]);
        lv_label_set_text(label, items[i].text);
        lv_obj_center(label);
    }
    
    updateSelection(model->getSelectedIndex());
    created = true;
    Serial.println("Menu View created");
}

void MenuView::destroy() {
    if (!created) return;
    
    if (screen) {
        lv_obj_del(screen);
        screen = nullptr;
    }
    title = nullptr;
    for(int i = 0; i < 4; i++) {
        buttons[i] = nullptr;
    }
    created = false;
    Serial.println("Menu View destroyed");
}

lv_obj_t* MenuView::getScreen() const {
    return screen;
}

bool MenuView::isCreated() const {
    return created;
}

void MenuView::updateSelection(int index) {
    if (!created) return;
    
    for(int i = 0; i < 4; i++) {
        if (buttons[i]) {
            if (i == index) {
                lv_obj_set_style_bg_color(buttons[i], lv_color_hex(0x0066CC), 0);
            } else {
                lv_obj_set_style_bg_color(buttons[i], lv_color_hex(0x333333), 0);
            }
        }
    }
}

void MenuView::button_event_cb(lv_event_t* e) {
    int index = (intptr_t)lv_event_get_user_data(e);
    Serial.printf("Menu item %d clicked\n", index);
}