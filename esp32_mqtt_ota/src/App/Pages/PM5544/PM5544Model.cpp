#include "PM5544Model.h"
#include <Arduino.h>
#include <algorithm>

// Static color definitions
const lv_color_t PM5544Model::PM5544_WHITE = lv_color_hex(0xFFFF);
const lv_color_t PM5544Model::PM5544_BLACK = lv_color_hex(0x0000);
const lv_color_t PM5544Model::PM5544_RED = lv_color_hex(0xF800);
const lv_color_t PM5544Model::PM5544_GREEN = lv_color_hex(0x07E0);
const lv_color_t PM5544Model::PM5544_BLUE = lv_color_hex(0x001F);
const lv_color_t PM5544Model::PM5544_YELLOW = lv_color_hex(0xFFE0);
const lv_color_t PM5544Model::PM5544_MAGENTA = lv_color_hex(0xF81F);
const lv_color_t PM5544Model::PM5544_CYAN = lv_color_hex(0x07FF);
const lv_color_t PM5544Model::PM5544_GRAY = lv_color_hex(0x8410);

PM5544Model::PM5544Model() {
    centerX = 160; // 320/2
    centerY = 120; // 240/2
    radius = std::min(centerX, centerY) - 10;
    barWidth = 320 / 8;
    barHeight = 25;
}

const lv_color_t* PM5544Model::getColors() const {
    static const lv_color_t colors[] = {
        PM5544_WHITE, PM5544_YELLOW, PM5544_CYAN, PM5544_GREEN,
        PM5544_MAGENTA, PM5544_RED, PM5544_BLUE, PM5544_BLACK
    };
    return colors;
}

const lv_color_t* PM5544Model::getQuadColors() const {
    static const lv_color_t quad_colors[] = {
        PM5544_RED, PM5544_GREEN, PM5544_BLUE, PM5544_WHITE
    };
    return quad_colors;
}

const lv_color_t* PM5544Model::getCenterColors() const {
    static const lv_color_t center_colors[] = {
        PM5544_WHITE, PM5544_BLACK, PM5544_WHITE, PM5544_BLACK, PM5544_WHITE
    };
    return center_colors;
}

const int* PM5544Model::getCenterSizes() const {
    static const int center_sizes[] = {40, 30, 20, 10, 4};
    return center_sizes;
}