#ifndef PAGE_MANAGER_H
#define PAGE_MANAGER_H

#include <lvgl.h>

// Forward declarations
class MenuModel;
class MenuView;
class MenuPresenter;
class SettingsModel;
class SettingsView;
class SettingsPresenter;
class StatusModel;
class StatusView;
class StatusPresenter;

// 頁面ID枚舉
enum PageID {
    PAGE_MENU = 0,
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
    MenuModel* menuModel;
    MenuView* menuView;
    MenuPresenter* menuPresenter;
    
    SettingsModel* settingsModel;
    SettingsView* settingsView;
    SettingsPresenter* settingsPresenter;
    
    StatusModel* statusModel;
    StatusView* statusView;
    StatusPresenter* statusPresenter;

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
    
    // 獲取 MVP 組件實例
    MenuPresenter* getMenuPresenter() const;
    SettingsPresenter* getSettingsPresenter() const;
    StatusPresenter* getStatusPresenter() const;
    
    // 清理所有頁面
    void cleanup();
    
private:
    void initializeComponents();
    void cleanupComponents();
};

#endif // PAGE_MANAGER_H