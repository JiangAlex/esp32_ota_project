#include "PageManager.h"
#include "App/Pages/Menu/MenuModel.h"
#include "App/Pages/Menu/MenuView.h"
#include "App/Pages/Menu/MenuPresenter.h"
#include "App/Pages/Settings/SettingsModel.h"
#include "App/Pages/Settings/SettingsView.h"
#include "App/Pages/Settings/SettingsPresenter.h"
#include "App/Pages/Status/StatusModel.h"
#include "App/Pages/Status/StatusView.h"
#include "App/Pages/Status/StatusPresenter.h"
#include <Arduino.h>

PageManager* PageManager::instance = nullptr;

PageManager::PageManager() : current_page(PAGE_MENU), previous_page(PAGE_MENU) {
    menuModel = nullptr;
    menuView = nullptr;
    menuPresenter = nullptr;
    settingsModel = nullptr;
    settingsView = nullptr;
    settingsPresenter = nullptr;
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
    
    // Initialize Settings MVP components
    settingsModel = new SettingsModel();
    settingsView = new SettingsView();
    settingsPresenter = new SettingsPresenter(settingsView, settingsModel);
    
    // Initialize Status MVP components
    statusModel = new StatusModel();
    statusView = new StatusView();
    statusPresenter = new StatusPresenter(statusView, statusModel);
    
    Serial.println("PageManager MVP components initialized");
}

void PageManager::init() {
    // 初始化時創建第一個頁面（Menu）
    switchToPage(PAGE_MENU);
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
        case PAGE_MENU:
            if (menuPresenter && menuView) {
                if (!menuView->isCreated()) {
                    menuPresenter->onCreate();
                }
                menuPresenter->onShow();
                screen = menuView->getScreen();
            }
            break;
            
        case PAGE_SETTINGS:
            if (settingsPresenter && settingsView) {
                if (!settingsView->isCreated()) {
                    settingsPresenter->onCreate();
                }
                settingsPresenter->onShow();
                screen = settingsView->getScreen();
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
        case PAGE_MENU: return "Menu";
        case PAGE_SETTINGS: return "Settings";
        case PAGE_STATUS: return "Status";
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
    
    // Cleanup Settings MVP components
    if (settingsPresenter) {
        settingsPresenter->onDestroy();
        delete settingsPresenter;
        settingsPresenter = nullptr;
    }
    if (settingsView) {
        delete settingsView;
        settingsView = nullptr;
    }
    if (settingsModel) {
        delete settingsModel;
        settingsModel = nullptr;
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

SettingsPresenter* PageManager::getSettingsPresenter() const {
    return settingsPresenter;
}

StatusPresenter* PageManager::getStatusPresenter() const {
    return statusPresenter;
}