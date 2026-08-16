/*
 * droidboot_herolte.c
 *
 * LVGL drivers for Samsung Exynos 8890 / herolte LK.
 */

#include <lvgl.h>
#include <droidboot_drivers.h>
#include <droidboot_logging.h>

#include <lib/bio.h>
#include <lib/partition.h>

#include <sys/types.h>
#include <kernel/thread.h>
#include <debug.h>

#include <platform.h>
#include <target.h>

#include <stdint.h>
#include <stdbool.h>

bool metadata_fail;
uint32_t last_pressed_key;


/*
 * --------------------------------------------------------------------------
 * Framebuffer
 * --------------------------------------------------------------------------
 */

static uint32_t *herolte_fb;
static uint32_t herolte_fb_width  = 1440;
static uint32_t herolte_fb_height = 2560;
static uint32_t herolte_fb_stride = 1440;

void droidboot_herolte_set_framebuffer(
        void *fb,
        uint32_t width,
        uint32_t height,
        uint32_t stride)
{
    herolte_fb = (uint32_t *)fb;
    herolte_fb_width = width;
    herolte_fb_height = height;
    herolte_fb_stride = stride;
}


static uint32_t lvgl_to_argb8888(lv_color_t color)
{
    return 0xff000000 |
           ((uint32_t)color.ch.red << 16) |
           ((uint32_t)color.ch.green << 8) |
           (uint32_t)color.ch.blue;
}


void droidboot_internal_fb_flush(
        lv_disp_drv_t *disp_drv,
        const lv_area_t *area,
        lv_color_t *color_p)
{
    int32_t x;
    int32_t y;

    if (!herolte_fb) {
        lv_disp_flush_ready(disp_drv);
        return;
    }

    for (y = area->y1; y <= area->y2; y++) {

        uint32_t *dst =
            herolte_fb +
            (y * herolte_fb_stride) +
            area->x1;

        lv_color_t *src =
            color_p +
            ((y - area->y1) *
             (area->x2 - area->x1 + 1));

        for (x = area->x1; x <= area->x2; x++) {

            *dst = lvgl_to_argb8888(*src);

            dst++;
            src++;
        }
    }

    /*
     * Samsung/Exynos framebuffer update goes here.
     *
     * Do not use:
     *
     *     mtkfb_draw_block()
     *     mt_disp_update()
     */

    lv_disp_flush_ready(disp_drv);
}


/*
 * --------------------------------------------------------------------------
 * Keys
 * --------------------------------------------------------------------------
 *
 * Replace these with the key definitions used by your herolte LK tree.
 */

#ifndef HEROLTE_KEY_VOLUME_UP
#define HEROLTE_KEY_VOLUME_UP     115
#endif

#ifndef HEROLTE_KEY_VOLUME_DOWN
#define HEROLTE_KEY_VOLUME_DOWN   114
#endif

#ifndef HEROLTE_KEY_POWER
#define HEROLTE_KEY_POWER         116
#endif


static bool herolte_key_pressed(uint32_t key)
{
    /*
     * Replace with the actual Samsung LK key API.
     */

    (void)key;

    return false;
}


bool droidboot_internal_key_read(
        lv_indev_drv_t *drv,
        lv_indev_data_t *data)
{
    (void)drv;

    if (herolte_key_pressed(HEROLTE_KEY_VOLUME_UP)) {

        data->key = LV_KEY_PREV;
        last_pressed_key = LV_KEY_PREV;
        data->state = LV_INDEV_STATE_PRESSED;

    } else if (herolte_key_pressed(HEROLTE_KEY_VOLUME_DOWN)) {

        data->key = LV_KEY_NEXT;
        last_pressed_key = LV_KEY_NEXT;
        data->state = LV_INDEV_STATE_PRESSED;

    } else if (herolte_key_pressed(HEROLTE_KEY_POWER)) {

        data->key = LV_KEY_ENTER;
        last_pressed_key = LV_KEY_ENTER;
        data->state = LV_INDEV_STATE_PRESSED;

    } else {

        data->key = last_pressed_key;
        data->state = LV_INDEV_STATE_RELEASED;
    }

    return false;
}


/*
 * --------------------------------------------------------------------------
 * SD card
 * --------------------------------------------------------------------------
 */

#ifndef HEROLTE_SD_BDEV
#define HEROLTE_SD_BDEV "sdmmc"
#endif


int droidboot_herolte_sd_card(void)
{
    /*
     * Replace with the SD initialization function from your LK tree
     * if initialization is not automatic.
     */

    return 0;
}


ssize_t droidboot_internal_sd_read_block(
        void *buf,
        uint32_t block,
        uint count)
{
    bdev_t *bdev;

    bdev = bio_open(HEROLTE_SD_BDEV);

    if (!bdev)
        return -1;

    return bio_read_block(bdev, buf, block, count);
}


ssize_t droidboot_internal_sd_write_block(
        const void *buf,
        uint32_t block,
        uint count)
{
    bdev_t *bdev;

    bdev = bio_open(HEROLTE_SD_BDEV);

    if (!bdev)
        return -1;

    return bio_write_block(bdev, buf, block, count);
}


uint32_t droidboot_internal_sd_blklen(void)
{
    bdev_t *bdev;

    bdev = bio_open(HEROLTE_SD_BDEV);

    if (!bdev)
        return 0;

    return bdev->block_size;
}


uint64_t droidboot_internal_sd_blkcnt(void)
{
    bdev_t *bdev;

    bdev = bio_open(HEROLTE_SD_BDEV);

    if (!bdev)
        return 0;

    return bdev->block_count;
}


bool droidboot_internal_sd_exists(void)
{
    bdev_t *bdev;

    bdev = bio_open(HEROLTE_SD_BDEV);

    return bdev != NULL;
}


/*
 * --------------------------------------------------------------------------
 * Display dimensions
 * --------------------------------------------------------------------------
 */

int droidboot_internal_get_display_height(void)
{
    return herolte_fb_height;
}


int droidboot_internal_get_display_width(void)
{
    return herolte_fb_width;
}


/*
 * --------------------------------------------------------------------------
 * Logging
 * --------------------------------------------------------------------------
 */

void droidboot_internal_platform_on_screen_log(const char *buf)
{
    printf("%s", buf);
}


void droidboot_internal_platform_system_log(const char *buf)
{
    printf("%s", buf);
}


/*
 * --------------------------------------------------------------------------
 * Initialization
 * --------------------------------------------------------------------------
 */

droidboot_error droidboot_internal_platform_init(void)
{
    droidboot_herolte_sd_card();

    return DROIDBOOT_EOK;
}


void droidboot_internal_delay(unsigned int time)
{
    thread_sleep(time);
}


/*
 * --------------------------------------------------------------------------
 * Linux boot
 * --------------------------------------------------------------------------
 */

void droidboot_internal_boot_linux_from_ram(
        void *kernel_raw,
        off_t kernel_raw_size,
        void *ramdisk_raw,
        off_t ramdisk_size,
        void *dtb_raw,
        off_t dtb_raw_size,
        void *dtbo_raw,
        off_t dtbo_raw_size,
        char *options)
{
    (void)ramdisk_raw;
    (void)dtb_raw;
    (void)dtb_raw_size;
    (void)dtbo_raw;
    (void)dtbo_raw_size;

    kcmdline_append(options);

    /*
     * Replace this with the boot_linux_from_ram() implementation
     * provided by your Exynos/LK tree.
     */

    boot_linux_from_ram(
        kernel_raw,
        kernel_raw_size,
        ramdisk_size);
}


void droidboot_internal_pre_ramdisk_load(
        void *kernel_raw,
        off_t kernel_raw_size)
{
    (void)kernel_raw;
    (void)kernel_raw_size;
}


void *droidboot_internal_get_kernel_load_addr(void)
{
    return NULL;
}


void *droidboot_internal_get_ramdisk_load_addr(void)
{
    return NULL;
}


bool droidboot_internal_append_ramdisk_to_kernel(void)
{
    return true;
}


void *droidboot_internal_get_dtb_load_addr(void)
{
    return NULL;
}


/*
 * --------------------------------------------------------------------------
 * Boot menu
 * --------------------------------------------------------------------------
 */

int exit_herolte;


static void droidboot_herolte_menu_event_handler(lv_event_t *e)
{
    lv_event_code_t code;
    lv_obj_t *obj;
    int index;

    code = lv_event_get_code(e);
    obj = lv_event_get_target(e);

    if (code != LV_EVENT_CLICKED)
        return;

    index = lv_obj_get_child_id(obj);

    switch (index) {

    case 0:
        /* Boot recovery */
        exit_herolte = 2;
        break;

    case 1:
        /* Fastboot */
        exit_herolte = 99;
        break;

    case 2:
        /* Normal boot */
        exit_herolte = 9;
        break;

    case 3:
        exit_herolte = 0;
        break;

    default:
        break;
    }
}


void droidboot_herolte_main_menu_add_options(lv_obj_t *list)
{
    lv_obj_t *list_btn;

    list_btn = lv_list_add_btn(
        list,
        NULL,
        "\nBoot recovery");

    lv_obj_add_event_cb(
        list_btn,
        droidboot_herolte_menu_event_handler,
        LV_EVENT_CLICKED,
        NULL);


    list_btn = lv_list_add_btn(
        list,
        NULL,
        "\nEnter fastboot menu");

    lv_obj_add_event_cb(
        list_btn,
        droidboot_herolte_menu_event_handler,
        LV_EVENT_CLICKED,
        NULL);


    list_btn = lv_list_add_btn(
        list,
        NULL,
        "\nNormal boot");

    lv_obj_add_event_cb(
        list_btn,
        droidboot_herolte_menu_event_handler,
        LV_EVENT_CLICKED,
        NULL);
}


int droidboot_herolte_show_boot_mode_menu(void)
{
    lv_obj_t *win;
    lv_obj_t *win_title;
    lv_obj_t *list1;

    exit_herolte = -1;

    win = lv_win_create(
        lv_scr_act(),
        lv_pct(6));

    lv_obj_set_pos(win, 0, 0);

    lv_obj_set_size(
        win,
        lv_pct(100),
        lv_pct(100));


    win_title = lv_win_add_title(
        win,
        "  Recovery Menu");

    lv_obj_set_pos(
        win_title,
        0,
        0);


    list1 = lv_list_create(win);

    lv_obj_set_size(
        list1,
        lv_pct(100),
        lv_pct(100));

    lv_obj_set_pos(
        list1,
        0,
        0);

    lv_obj_align(
        list1,
        LV_ALIGN_BOTTOM_MID,
        0,
        0);


    droidboot_herolte_main_menu_add_options(list1);


    while (exit_herolte == -1)
        thread_sleep(50);

    return exit_herolte;
}
