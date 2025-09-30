#ifndef STATUS_VIEW_H
#define STATUS_VIEW_H

#include "../../MVP/BaseView.h"

class StatusView : public BaseView {
private:
    lv_obj_t* screen;
    bool created;
    lv_obj_t* statusLabel;

public:
    StatusView();
    ~StatusView();
    
    void create() override;
    void destroy() override;
    lv_obj_t* getScreen() const override;
    bool isCreated() const override;
    
    void updateStatus(const char* status);
};

#endif // STATUS_VIEW_H