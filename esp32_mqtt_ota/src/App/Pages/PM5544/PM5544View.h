#ifndef PM5544_VIEW_H
#define PM5544_VIEW_H

#include "../../MVP/BaseView.h"
#include "PM5544Model.h"

class PM5544View : public BaseView {
private:
    lv_obj_t* screen;
    bool created;
    PM5544Model* model;

public:
    PM5544View(PM5544Model* m);
    ~PM5544View();
    
    void create() override;
    void destroy() override;
    lv_obj_t* getScreen() const override;
    bool isCreated() const override;
    
private:
    void createColorBars();
    void createConcentricCircles();
    void createCrossHairs();
    void createDiagonalLines();
    void createQuadrantCircles();
    void createCenterTarget();
};

#endif // PM5544_VIEW_H