#ifndef BASE_VIEW_H
#define BASE_VIEW_H

#include <lvgl.h>

class BaseView {
public:
    virtual ~BaseView() = default;
    virtual void create() = 0;
    virtual void destroy() = 0;
    virtual lv_obj_t* getScreen() const = 0;
    virtual bool isCreated() const = 0;
};

#endif // BASE_VIEW_H