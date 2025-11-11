#include "PageManager.h"
#include "App/Pages/Menu/MenuModel.h"
#include "App/Pages/Menu/MenuView.h"
#include "App/Pages/Menu/MenuPresenter.h"
// Settings頁面已移除
// #include "App/Pages/Settings/SettingsModel.h"
// #include "App/Pages/Settings/SettingsView.h"
// #include "App/Pages/Settings/SettingsPresenter.h"
// #include "App/Pages/Trekking/TrekkingModel.h"
#include "App/Pages/Trekking/TrekkingView.h"
// #include "App/Pages/Trekking/TrekkingPresenter.h"
#include "App/Pages/WalkieTalkie/WalkieTalkieView.h"
#include "App/Pages/System/SystemView.h"
#include "App/Pages/Status/StatusModel.h"
#include "App/Pages/Status/StatusView.h"
#include "App/Pages/Status/StatusPresenter.h"
#include <Arduino.h>

PageManager* PageManager::instance = nullptr;

PageManager::PageManager() : current_page(PAGE_MAINMENU), previous_page(PAGE_MAINMENU) {
    menuModel = nullptr;
    menuView = nullptr;
    menuPresenter = nullptr;
    // Settings相關組件已移除
    // settingsModel = nullptr;
    // settingsView = nullptr;
    // settingsPresenter = nullptr;
    // 新頁面組件初始化
    // trekkingModel = nullptr;
    trekkingView = nullptr;
    // trekkingPresenter = nullptr;
    walkieTalkieView = nullptr;
    systemView = nullptr;
    // Status組件
    statusModel = nullptr;
    statusView = nullptr;
    statusPresenter = nullptr;
}

PageManager::~PageManager() {
    cleanup();
}

PageManager* PageManager::getInstance() {
    if (instance == nullptr) {
        instance = new PageManager();
        instance->initializeComponents();
    }
    return instance;
}

void PageManager::deleteInstance() {
    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
    }
}

void PageManager::initializeComponents() {
    // Initialize Menu MVP components
    menuModel = new MenuModel();
    menuView = new MenuView(menuModel);
    menuPresenter = new MenuPresenter(menuView, menuModel);
    
    // Settings頁面已移除，以後可以用於新的頁面
    // settingsModel = new SettingsModel();
    // settingsView = new SettingsView();
    // settingsPresenter = new SettingsPresenter(settingsView, settingsModel);
    
    // Initialize Trekking MVP components - temporarily disabled
    // trekkingModel = new TrekkingModel();
    trekkingView = new TrekkingView();
    // trekkingPresenter = new TrekkingPresenter(trekkingView, trekkingModel);
    
    // Initialize simple view-only pages
    walkieTalkieView = new WalkieTalkieView();
    systemView = new SystemView();
    
    // Initialize Status MVP components
    statusModel = new StatusModel();
    statusView = new StatusView();
    statusPresenter = new StatusPresenter(statusView, statusModel);
    
    Serial.println("PageManager MVP components initialized");
}

void PageManager::init() {
    // 初始化時創建第一個頁面（MainMenu）
    switchToPage(PAGE_MAINMENU);
}

bool PageManager::switchToPage(PageID pageId) {
    if (pageId >= PAGE_COUNT || pageId < 0) {
        return false;
    }
    
    // 記錄前一個頁面
    previous_page = current_page;
    current_page = pageId;
    
    lv_obj_t* screen = nullptr;
    
    switch (pageId) {
        case PAGE_MAINMENU:
            if (menuPresenter && menuView) {
                if (!menuView->isCreated()) {
                    menuPresenter->onCreate();
                }
                menuPresenter->onShow();
                screen = menuView->getScreen();
            }
            break;
            
        case PAGE_TREKKING:
            if (trekkingView) {
                if (!trekkingView->isCreated()) {
                    trekkingView->create();
                }
                screen = trekkingView->getScreen();
            }
            break;
            
        case PAGE_WALKIETALKIE:
            if (walkieTalkieView) {
                if (!walkieTalkieView->isCreated()) {
                    walkieTalkieView->create();
                }
                screen = walkieTalkieView->getScreen();
            }
            break;
            
        case PAGE_SYSTEM:
            if (systemView) {
                if (!systemView->isCreated()) {
                    systemView->create();
                }
                screen = systemView->getScreen();
            }
            break;
            
        case PAGE_STATUS:
            if (statusPresenter && statusView) {
                if (!statusView->isCreated()) {
                    statusPresenter->onCreate();
                }
                statusPresenter->onShow();
                screen = statusView->getScreen();
            }
            break;
    }
    
    if (screen) {
        lv_scr_load(screen);
        Serial.printf("Switched to page: %s\n", getPageName(pageId));
        return true;
    }
    
    Serial.printf("Failed to switch to page: %s\n", getPageName(pageId));
    return false;
}

bool PageManager::switchToNextPage() {
    PageID nextPage = (PageID)((current_page + 1) % PAGE_COUNT);
    return switchToPage(nextPage);
}

bool PageManager::switchToPreviousPage() {
    PageID prevPage = (PageID)((current_page - 1 + PAGE_COUNT) % PAGE_COUNT);
    return switchToPage(prevPage);
}

PageID PageManager::getCurrentPage() const {
    return current_page;
}

PageID PageManager::getPreviousPage() const {
    return previous_page;
}

const char* PageManager::getPageName(PageID pageId) const {
    switch (pageId) {
        case PAGE_MAINMENU: return "MainMenu";
        case PAGE_TREKKING: return "Trekking";
        case PAGE_WALKIETALKIE: return "WalkieTalkie";
        case PAGE_STATUS: return "Status";
        case PAGE_SYSTEM: return "System";
        default: return "Unknown";
    }
}

void PageManager::cleanup() {
    cleanupComponents();
}

void PageManager::cleanupComponents() {
    // Cleanup Menu MVP components
    if (menuPresenter) {
        menuPresenter->onDestroy();
        delete menuPresenter;
        menuPresenter = nullptr;
    }
    if (menuView) {
        delete menuView;
        menuView = nullptr;
    }
    if (menuModel) {
        delete menuModel;
        menuModel = nullptr;
    }
    
    // Settings組件已移除
    // if (settingsPresenter) {
    //     settingsPresenter->onDestroy();
    //     delete settingsPresenter;
    //     settingsPresenter = nullptr;
    // }
    // if (settingsView) {
    //     delete settingsView;
    //     settingsView = nullptr;
    // }
    // if (settingsModel) {
    //     delete settingsModel;
    //     settingsModel = nullptr;
    // }
    
    // Cleanup Trekking MVP components
    // if (trekkingPresenter) {
    //     trekkingPresenter->onDestroy();
    //     delete trekkingPresenter;
    //     trekkingPresenter = nullptr;
    // }
    if (trekkingView) {
        delete trekkingView;
        trekkingView = nullptr;
    }
    // if (trekkingModel) {
    //     delete trekkingModel;
    //     trekkingModel = nullptr;
    // }
    
    // Cleanup view-only pages
    if (walkieTalkieView) {
        delete walkieTalkieView;
        walkieTalkieView = nullptr;
    }
    if (systemView) {
        delete systemView;
        systemView = nullptr;
    }
    
    // Cleanup Status MVP components
    if (statusPresenter) {
        statusPresenter->onDestroy();
        delete statusPresenter;
        statusPresenter = nullptr;
    }
    if (statusView) {
        delete statusView;
        statusView = nullptr;
    }
    if (statusModel) {
        delete statusModel;
        statusModel = nullptr;
    }
    
    Serial.println("PageManager MVP components cleaned up");
}

// 獲取 MVP 組件實例
MenuPresenter* PageManager::getMenuPresenter() const {
    return menuPresenter;
}

// Settings getter已移除
// SettingsPresenter* PageManager::getSettingsPresenter() const {
//     return settingsPresenter;
// }

// TrekkingPresenter* PageManager::getTrekkingPresenter() const {
//     return trekkingPresenter;
// }

StatusPresenter* PageManager::getStatusPresenter() const {
    return statusPresenter;
}

TrekkingView* PageManager::getTrekkingView() const {
    return trekkingView;
}

WalkieTalkieView* PageManager::getWalkieTalkieView() const {
    return walkieTalkieView;
}

SystemView* PageManager::getSystemView() const {
    return systemView;
}