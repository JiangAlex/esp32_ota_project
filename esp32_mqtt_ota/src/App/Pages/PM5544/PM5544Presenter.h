#ifndef PM5544_PRESENTER_H
#define PM5544_PRESENTER_H

#include "../../MVP/BasePresenter.h"
#include "PM5544View.h"
#include "PM5544Model.h"

class PM5544Presenter : public BasePresenter<PM5544View> {
private:
    PM5544Model* model;

public:
    PM5544Presenter(PM5544View* view, PM5544Model* model);
    ~PM5544Presenter();
    
    void onCreate() override;
    void onDestroy() override;
    void onShow() override;
    void onHide() override;
    
    // Test pattern specific methods
    void refreshPattern();
};

#endif // PM5544_PRESENTER_H