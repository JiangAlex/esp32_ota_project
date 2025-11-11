/**
 * @file lv_conf_backup.h
 * 備用配置 - 如果1位色深有問題時使用16位色深的替代方案
 * 使用方法：將此文件重命名為lv_conf.h來替換當前配置
 */

#ifndef LV_CONF_BACKUP_H
#define LV_CONF_BACKUP_H

#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0
#define LV_MEM_SIZE (48U * 1024U)
#define LV_DISP_DEF_REFR_PERIOD 30
#define LV_INDEV_DEF_READ_PERIOD 30
#define LV_TICK_CUSTOM 1
#define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())
#define LV_DPI_DEF 96  // 降低DPI以改善點陣顯示

#define LV_USE_ANIMATION 1
#define LV_USE_SHADOW 0  // 關閉陰影以改善性能
#define LV_USE_BLEND_MODES 0  // 關閉混合模式
#define LV_USE_OPA_SCALE 1
#define LV_USE_IMG_TRANSFORM 0  // 關閉圖像變換
#define LV_USE_GROUP 1
#define LV_USE_GPU 0
#define LV_USE_FILESYSTEM 0
#define LV_USE_USER_DATA 1

#define LV_USE_LOG 0
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ 0
#define LV_USE_ASSERT_STYLE 0

#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_DEFAULT &lv_font_montserrat_12
#define LV_FONT_MONTSERRAT_20 0  // 關閉大字體節省記憶體

// 字體渲染優化設定 - 針對單色OLED優化
#define LV_FONT_FMT_TXT_LARGE 0
#define LV_USE_FONT_PLACEHOLDER 1
#define LV_FONT_SUBPX 0  // 關閉次像素渲染
#define LV_USE_FONT_COMPRESSED 0

#define LV_USE_ARC 1
#define LV_USE_BAR 1
#define LV_USE_BTN 1
#define LV_USE_BTNMATRIX 1
#define LV_USE_CANVAS 0  // 關閉Canvas節省記憶體
#define LV_USE_CHECKBOX 1
#define LV_USE_DROPDOWN 1
#define LV_USE_IMG 1
#define LV_USE_LABEL 1
#define LV_USE_LINE 1
#define LV_USE_ROLLER 1
#define LV_USE_SLIDER 1
#define LV_USE_SWITCH 1
#define LV_USE_TEXTAREA 1
#define LV_USE_TABLE 1

#endif /*LV_CONF_BACKUP_H*/