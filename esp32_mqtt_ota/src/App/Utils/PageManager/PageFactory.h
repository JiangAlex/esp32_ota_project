#ifndef PAGE_FACTORY_H
#define PAGE_FACTORY_H

#include "PageManager.h"

class PageFactory {
public:
    // 創建 PM5544 頁面組件
    static void createPM5544Components(PM5544Model*& model, PM5544View*& view, PM5544Presenter*& presenter);
    
    // 創建 Menu 頁面組件
    static void createMenuComponents(MenuModel*& model, MenuView*& view, MenuPresenter*& presenter);
    
    // 清理組件
    static void cleanupPM5544Components(PM5544Model* model, PM5544View* view, PM5544Presenter* presenter);
    static void cleanupMenuComponents(MenuModel* model, MenuView* view, MenuPresenter* presenter);
};

#endif // PAGE_FACTORY_H