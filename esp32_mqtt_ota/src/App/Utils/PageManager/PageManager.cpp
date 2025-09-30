#include "PageManager.h"
#include "App/Pages/PM5544/PM5544Model.h"
#include "App/Pages/PM5544/PM5544View.h"
#include "App/Pages/PM5544/PM5544Presenter.h"
#include "App/Pages/Menu/MenuModel.h"
#include "App/Pages/Menu/MenuView.h"
#include "App/Pages/Menu/MenuPresenter.h"
#include "App/Pages/Settings/SettingsView.h"
#include "App/Pages/Status/StatusView.h"
#include <Arduino.h>

PageManager* PageManager::instance = nullptr;

PageManager::PageManager() : current_page(PAGE_PM5544), previous_page(PAGE_PM5544) {
    pm5544Model = nullptr;
    pm5544View = nullptr;
    pm5544Presenter = nullptr;
    menuModel = nullptr;
    menuView = nullptr;
    menuPresenter = nullptr;
    settingsView = nullptr;
    statusView = nullptr;
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
    // Initialize MVP components
    pm5544Model = new PM5544Model();
    pm5544View = new PM5544View(pm5544Model);
    pm5544Presenter = new PM5544Presenter(pm5544View, pm5544Model);
    
    menuModel = new MenuModel();
    menuView = new MenuView(menuModel);
    menuPresenter = new MenuPresenter(menuView, menuModel);
    
    settingsView = new SettingsView();
    statusView = new StatusView();
    
    Serial.println("PageManager components initialized");
}

void PageManager::init() {
    // 初始化時創建第一個頁面
    switchToPage(PAGE_PM5544);
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
        case PAGE_PM5544:
            if (pm5544Presenter && pm5544View) {
                if (!pm5544View->isCreated()) {
                    pm5544Presenter->onCreate();
                }
                pm5544Presenter->onShow();
                screen = pm5544View->getScreen();
            }
            break;
            
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
            if (settingsView) {
                if (!settingsView->isCreated()) {
                    settingsView->create();
                }
                screen = settingsView->getScreen();
            }
            break;
            
        case PAGE_STATUS:
            if (statusView) {
                if (!statusView->isCreated()) {
                    statusView->create();
                }
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
        case PAGE_PM5544: return "PM5544 Test";
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
    if (pm5544Presenter) {
        pm5544Presenter->onDestroy();
        delete pm5544Presenter;
        pm5544Presenter = nullptr;
    }
    if (pm5544View) {
        delete pm5544View;
        pm5544View = nullptr;
    }
    if (pm5544Model) {
        delete pm5544Model;
        pm5544Model = nullptr;
    }
    
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
    
    if (settingsView) {
        delete settingsView;
        settingsView = nullptr;
    }
    if (statusView) {
        delete statusView;
        statusView = nullptr;
    }
    
    Serial.println("PageManager components cleaned up");
}