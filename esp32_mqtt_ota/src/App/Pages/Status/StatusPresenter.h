#ifndef STATUS_PRESENTER_H
#define STATUS_PRESENTER_H

#include "../../MVP/BasePresenter.h"
#include "StatusView.h"
#include "StatusModel.h"

class StatusPresenter : public BasePresenter<StatusView> {
private:
    StatusModel* model;

public:
    StatusPresenter(StatusView* view, StatusModel* model);
    ~StatusPresenter();
    
    void onCreate() override;
    void onDestroy() override;
    void onShow() override;
    void onHide() override;
    
    // 業務邏輯方法
    void updateStatus();
    void scrollUp();
    void scrollDown();
};

#endif // STATUS_PRESENTER_H