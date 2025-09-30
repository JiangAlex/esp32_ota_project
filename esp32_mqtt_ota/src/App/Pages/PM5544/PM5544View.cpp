#include "PM5544View.h"
#include <Arduino.h>

PM5544View::PM5544View(PM5544Model* m) : model(m), screen(nullptr), created(false) {}

PM5544View::~PM5544View() {
    destroy();
}

void PM5544View::create() {
    if (created) return;
    
    screen = lv_obj_create(nullptr);
    
    // Set screen background to black
    lv_obj_set_style_bg_color(screen, PM5544Model::PM5544_BLACK, 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    
    createColorBars();
    createConcentricCircles();
    createCrossHairs();
    createDiagonalLines();
    createQuadrantCircles();
    createCenterTarget();
    
    created = true;
    Serial.println("PM5544 View created");
}

void PM5544View::destroy() {
    if (!created) return;
    
    if (screen) {
        lv_obj_del(screen);
        screen = nullptr;
    }
    created = false;
    Serial.println("PM5544 View destroyed");
}

lv_obj_t* PM5544View::getScreen() const {
    return screen;
}

bool PM5544View::isCreated() const {
    return created;
}

void PM5544View::createColorBars() {
    const lv_color_t* colors = model->getColors();
    
    // Top color bars
    for(int i = 0; i < 8; i++) {
        lv_obj_t *rect = lv_obj_create(screen);
        lv_obj_set_size(rect, model->getBarWidth(), model->getBarHeight());
        lv_obj_set_pos(rect, i * model->getBarWidth(), 0);
        lv_obj_set_style_bg_color(rect, colors[i], 0);
        lv_obj_set_style_bg_opa(rect, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(rect, 0, 0);
        lv_obj_set_style_radius(rect, 0, 0);
    }
    
    // Bottom color bars (reversed)
    for(int i = 0; i < 8; i++) {
        lv_obj_t *rect = lv_obj_create(screen);
        lv_obj_set_size(rect, model->getBarWidth(), model->getBarHeight());
        lv_obj_set_pos(rect, i * model->getBarWidth(), 240 - model->getBarHeight());
        lv_obj_set_style_bg_color(rect, colors[7-i], 0);
        lv_obj_set_style_bg_opa(rect, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(rect, 0, 0);
        lv_obj_set_style_radius(rect, 0, 0);
    }
}

void PM5544View::createConcentricCircles() {
    for(int r = model->getRadius(); r > 30; r -= 15) {
        lv_obj_t *arc = lv_arc_create(screen);
        lv_obj_set_size(arc, r * 2, r * 2);
        lv_obj_set_pos(arc, model->getCenterX() - r, model->getCenterY() - r);
        lv_arc_set_range(arc, 0, 360);
        lv_arc_set_value(arc, 360);
        lv_obj_set_style_arc_color(arc, PM5544Model::PM5544_WHITE, LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(arc, 2, LV_PART_INDICATOR);
        lv_obj_set_style_bg_opa(arc, LV_OPA_TRANSP, 0);
        lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
        lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
    }
}

void PM5544View::createCrossHairs() {
    int centerX = model->getCenterX();
    int centerY = model->getCenterY();
    int radius = model->getRadius();
    
    // Horizontal line
    lv_obj_t *h_line = lv_line_create(screen);
    static lv_point_t h_points[] = {{centerX - radius, centerY}, {centerX + radius, centerY}};
    lv_line_set_points(h_line, h_points, 2);
    lv_obj_set_style_line_color(h_line, PM5544Model::PM5544_WHITE, 0);
    lv_obj_set_style_line_width(h_line, 2, 0);
    
    // Vertical line
    lv_obj_t *v_line = lv_line_create(screen);
    static lv_point_t v_points[] = {{centerX, centerY - radius}, {centerX, centerY + radius}};
    lv_line_set_points(v_line, v_points, 2);
    lv_obj_set_style_line_color(v_line, PM5544Model::PM5544_WHITE, 0);
    lv_obj_set_style_line_width(v_line, 2, 0);
}

void PM5544View::createDiagonalLines() {
    int centerX = model->getCenterX();
    int centerY = model->getCenterY();
    int diagRadius = model->getRadius() * 0.8;
    
    lv_obj_t *d1_line = lv_line_create(screen);
    static lv_point_t d1_points[] = {{centerX - diagRadius, centerY - diagRadius}, 
                              {centerX + diagRadius, centerY + diagRadius}};
    lv_line_set_points(d1_line, d1_points, 2);
    lv_obj_set_style_line_color(d1_line, PM5544Model::PM5544_WHITE, 0);
    lv_obj_set_style_line_width(d1_line, 2, 0);
    
    lv_obj_t *d2_line = lv_line_create(screen);
    static lv_point_t d2_points[] = {{centerX - diagRadius, centerY + diagRadius}, 
                              {centerX + diagRadius, centerY - diagRadius}};
    lv_line_set_points(d2_line, d2_points, 2);
    lv_obj_set_style_line_color(d2_line, PM5544Model::PM5544_WHITE, 0);
    lv_obj_set_style_line_width(d2_line, 2, 0);
}

void PM5544View::createQuadrantCircles() {
    int centerX = model->getCenterX();
    int centerY = model->getCenterY();
    int quadRadius = model->getRadius() * 0.6;
    int colorCircleSize = 24;
    const lv_color_t* quad_colors = model->getQuadColors();
    
    int positions[][2] = {{centerX - quadRadius, centerY - quadRadius},  // Red (top-left)
                         {centerX + quadRadius, centerY - quadRadius},   // Green (top-right)
                         {centerX - quadRadius, centerY + quadRadius},   // Blue (bottom-left)
                         {centerX + quadRadius, centerY + quadRadius}};  // White (bottom-right)
    
    for(int i = 0; i < 4; i++) {
        lv_obj_t *circle = lv_obj_create(screen);
        lv_obj_set_size(circle, colorCircleSize, colorCircleSize);
        lv_obj_set_pos(circle, positions[i][0] - colorCircleSize/2, positions[i][1] - colorCircleSize/2);
        lv_obj_set_style_bg_color(circle, quad_colors[i], 0);
        lv_obj_set_style_bg_opa(circle, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(circle, 1, 0);
        lv_obj_set_style_border_color(circle, PM5544Model::PM5544_WHITE, 0);
        lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
    }
}

void PM5544View::createCenterTarget() {
    int centerX = model->getCenterX();
    int centerY = model->getCenterY();
    const int* center_sizes = model->getCenterSizes();
    const lv_color_t* center_colors = model->getCenterColors();
    
    for(int i = 0; i < 5; i++) {
        lv_obj_t *center = lv_obj_create(screen);
        lv_obj_set_size(center, center_sizes[i], center_sizes[i]);
        lv_obj_set_pos(center, centerX - center_sizes[i]/2, centerY - center_sizes[i]/2);
        lv_obj_set_style_bg_color(center, center_colors[i], 0);
        lv_obj_set_style_bg_opa(center, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(center, 0, 0);
        lv_obj_set_style_radius(center, LV_RADIUS_CIRCLE, 0);
    }
}