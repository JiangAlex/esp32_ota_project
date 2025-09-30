#ifndef BASE_PRESENTER_H
#define BASE_PRESENTER_H

template<typename View>
class BasePresenter {
protected:
    View* view;

public:
    BasePresenter(View* v) : view(v) {}
    virtual ~BasePresenter() = default;
    
    virtual void onCreate() = 0;
    virtual void onDestroy() = 0;
    virtual void onShow() {}
    virtual void onHide() {}
};

#endif // BASE_PRESENTER_H