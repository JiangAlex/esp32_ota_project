#ifndef PM5544_MODEL_H
#define PM5544_MODEL_H

#include "../../MVP/BaseModel.h"
#include <lvgl.h>

class PM5544Model : public BaseModel {
private:
    int centerX;
    int centerY;
    int radius;
    int barWidth;
    int barHeight;
    
public:
    PM5544Model();
    
    // PM5544 Test Pattern Colors
    static const lv_color_t PM5544_WHITE;
    static const lv_color_t PM5544_BLACK;
    static const lv_color_t PM5544_RED;
    static const lv_color_t PM5544_GREEN;
    static const lv_color_t PM5544_BLUE;
    static const lv_color_t PM5544_YELLOW;
    static const lv_color_t PM5544_MAGENTA;
    static const lv_color_t PM5544_CYAN;
    static const lv_color_t PM5544_GRAY;
    
    // Getters
    int getCenterX() const { return centerX; }
    int getCenterY() const { return centerY; }
    int getRadius() const { return radius; }
    int getBarWidth() const { return barWidth; }
    int getBarHeight() const { return barHeight; }
    
    // Color arrays
    const lv_color_t* getColors() const;
    const lv_color_t* getQuadColors() const;
    const lv_color_t* getCenterColors() const;
    const int* getCenterSizes() const;
};

#endif // PM5544_MODEL_H