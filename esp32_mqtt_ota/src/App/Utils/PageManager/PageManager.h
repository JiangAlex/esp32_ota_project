#ifndef PAGE_MANAGER_H
#define PAGE_MANAGER_H

#include <lvgl.h>

// Forward declarations
class PM5544Model;
class PM5544View;
class PM5544Presenter;
class MenuModel;
class MenuView;
class MenuPresenter;
class SettingsView;
class StatusView;

// 頁面ID枚舉
enum PageID {
    PAGE_PM5544 = 0,
    PAGE_MENU,
    PAGE_SETTINGS,
    PAGE_STATUS,
    PAGE_COUNT  // 總頁面數
};

class PageManager {
private:
    static PageManager* instance;
    PageID current_page;
    PageID previous_page;
    
    // MVP Components - 使用指標以避免包含所有標頭檔
    PM5544Model* pm5544Model;
    PM5544View* pm5544View;
    PM5544Presenter* pm5544Presenter;
    
    MenuModel* menuModel;
    MenuView* menuView;
    MenuPresenter* menuPresenter;
    
    SettingsView* settingsView;
    StatusView* statusView;

    // Private constructor for singleton
    PageManager();

public:
    ~PageManager();
    
    // 單例模式
    static PageManager* getInstance();
    static void deleteInstance();
    
    // 頁面管理
    void init();
    bool switchToPage(PageID pageId);
    bool switchToNextPage();
    bool switchToPreviousPage();
    PageID getCurrentPage() const;
    PageID getPreviousPage() const;
    const char* getPageName(PageID pageId) const;
    
    // 清理所有頁面
    void cleanup();
    
private:
    void initializeComponents();
    void cleanupComponents();
};

#endif // PAGE_MANAGER_H