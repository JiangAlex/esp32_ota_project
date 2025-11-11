#ifndef PAGE_MANAGER_H
#define PAGE_MANAGER_H

#include <lvgl.h>

// Forward declarations
class MenuModel;
class MenuView;
class MenuPresenter;
// Settings相關類別已移除
// class SettingsModel;
// class SettingsView;
// class SettingsPresenter;
// class TrekkingModel;
class TrekkingView;
// class TrekkingPresenter;
class WalkieTalkieView;
class SystemView;
class StatusModel;
class StatusView;
class StatusPresenter;

// 頁面ID枚舉
enum PageID {
    PAGE_MAINMENU = 0,
    PAGE_TREKKING,
    PAGE_WALKIETALKIE,
    PAGE_STATUS,
    PAGE_SYSTEM,
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
    
        // Settings相關組件已移除
    // SettingsModel* settingsModel;
    // SettingsView* settingsView;
    // SettingsPresenter* settingsPresenter;
    
    // 新增頁面組件
    // TrekkingModel* trekkingModel;
    TrekkingView* trekkingView;
    // TrekkingPresenter* trekkingPresenter;
    
    WalkieTalkieView* walkieTalkieView;
    SystemView* systemView;
    
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
    // SettingsPresenter* getSettingsPresenter() const; // Settings已移除
    // TrekkingPresenter* getTrekkingPresenter() const; // 暫時停用
    StatusPresenter* getStatusPresenter() const;
    
    // 獲取 View 組件實例（用於滾動功能）
    TrekkingView* getTrekkingView() const;
    WalkieTalkieView* getWalkieTalkieView() const;
    SystemView* getSystemView() const;
    
    // 清理所有頁面
    void cleanup();
    
private:
    void initializeComponents();
    void cleanupComponents();
};

#endif // PAGE_MANAGER_H