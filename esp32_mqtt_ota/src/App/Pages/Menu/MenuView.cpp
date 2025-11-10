#include "MenuView.h"
#include "../../Utils/OLEDLayout.h"
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
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    
    // 禁用主螢幕滾動條
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
    
    // 使用OLED布局創建頂部標題欄（單色 OLED 上區域 18px）
    lv_obj_t* topBar = OLEDLayout::createTopBar(screen, "MENU");
    
    // 創建內容區域（單色 OLED 下區域 46px）
    lv_obj_t* contentArea = OLEDLayout::createContentArea(screen);
    
    // Create menu buttons in content area (適應48px高度)
    const MenuItem* items = model->getMenuItems();
    int itemCount = model->getMenuItemCount();
    
    // 計算按鈕佈局 - 垂直排列（上下排列）
    if (itemCount <= 4) {
        for(int i = 0; i < itemCount && i < 4; i++) {
            buttons[i] = lv_btn_create(contentArea);
            
            // 按鈕尺寸適應OLED - 較寬的按鈕用於垂直排列
            lv_obj_set_size(buttons[i], 120, 18);
            
            // 垂直佈局 - 每個按鈕佔一行
            lv_obj_set_pos(buttons[i], 4, i * 22);  // 4px左邊距，每行間隔22px
            
            // 設置按鈕初始樣式 - 透明背景，細邊框
            lv_obj_set_style_bg_opa(buttons[i], LV_OPA_TRANSP, 0);
            lv_obj_set_style_border_width(buttons[i], 1, 0);
            lv_obj_set_style_border_color(buttons[i], lv_color_hex(0x666666), 0);
            lv_obj_set_style_radius(buttons[i], 4, 0); // 圓角邊框
            
            lv_obj_add_event_cb(buttons[i], button_event_cb, LV_EVENT_CLICKED, (void*)(intptr_t)i);
            
            lv_obj_t *label = lv_label_create(buttons[i]);
            lv_label_set_text(label, items[i].text);
            lv_obj_set_style_text_color(label, lv_color_white(), 0); // 白色文字
            
            // 使用 14px 字體以適應按鈕尺寸
            lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
            lv_obj_set_style_text_opa(label, LV_OPA_COVER, 0);
            lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
            
            lv_obj_center(label);
        }
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
                // 選中: 藍色框 (0x0066CC)
                lv_obj_set_style_bg_color(buttons[i], lv_color_hex(0x0066CC), 0);
                lv_obj_set_style_bg_opa(buttons[i], LV_OPA_COVER, 0);
                lv_obj_set_style_border_width(buttons[i], 2, 0);
                lv_obj_set_style_border_color(buttons[i], lv_color_hex(0x0066CC), 0);
            } else {
                // 未選中: 沒有背景，只有邊框
                lv_obj_set_style_bg_opa(buttons[i], LV_OPA_TRANSP, 0);
                lv_obj_set_style_border_width(buttons[i], 1, 0);
                lv_obj_set_style_border_color(buttons[i], lv_color_hex(0x666666), 0);
            }
            
            // 所有按鈕的文字都是白色並居中
            lv_obj_t* label = lv_obj_get_child(buttons[i], 0);
            if (label) {
                lv_obj_set_style_text_color(label, lv_color_white(), 0);
            }
        }
    }
}

void MenuView::button_event_cb(lv_event_t* e) {
    int index = (intptr_t)lv_event_get_user_data(e);
    Serial.printf("Menu item %d clicked\n", index);
}