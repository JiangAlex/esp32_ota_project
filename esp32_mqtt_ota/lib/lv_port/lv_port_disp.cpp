#include "lv_port_disp.h"
#include <lvgl.h>
#include "LGFX_ChappieCore.hpp"

/*********************
 *      DEFINES
 *********************/
#define DISP_BUF_SIZE (320 * 10)  // 直接使用具體數值而不是 LV_HOR_RES_MAX

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);

/**********************
 *  STATIC VARIABLES
 **********************/
static LGFX gfx;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/
    static lv_disp_draw_buf_t draw_buf_dsc;
    static lv_color_t buf_1[DISP_BUF_SIZE];                          /*A buffer for 10 rows*/
    static lv_color_t buf_2[DISP_BUF_SIZE];                          /*An optional buffer for 10 rows*/
    lv_disp_draw_buf_init(&draw_buf_dsc, buf_1, buf_2, DISP_BUF_SIZE);   /*Initialize the display buffer*/

    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/
    static lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set up the functions to access to your display*/
    
    /*Set the resolution of the display*/
    disp_drv.hor_res = 320;      // 旋轉後的水平解析度
    disp_drv.ver_res = 240;      // 旋轉後的垂直解析度

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
    gfx.init();
    gfx.setRotation(1);  // 設定旋轉90度
    gfx.setBrightness(128);
}

/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    gfx.startWrite();
    gfx.setAddrWindow(area->x1, area->y1, w, h);
    gfx.pushColors((uint16_t*)&color_p->full, w * h, true);
    gfx.endWrite();

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}