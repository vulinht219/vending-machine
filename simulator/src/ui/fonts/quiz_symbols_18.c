/*******************************************************************************
 * Size: 18 px
 * Bpp: 4
 * Opts: --font C:/Windows/Fonts/arial.ttf --size 18 --bpp 4 --format lvgl --symbols €£°×÷ --no-compress -o src/ui/fonts/quiz_symbols_18.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef QUIZ_SYMBOLS_18
#define QUIZ_SYMBOLS_18 1
#endif

#if QUIZ_SYMBOLS_18

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+00A3 "£" */
    0x0, 0x19, 0xef, 0xc5, 0x0, 0x1, 0xef, 0x86,
    0xcf, 0x60, 0x8, 0xf4, 0x0, 0xc, 0xe0, 0xa,
    0xf0, 0x0, 0x4, 0x60, 0x9, 0xf1, 0x0, 0x0,
    0x0, 0x7, 0xf6, 0x11, 0x0, 0x0, 0x9f, 0xff,
    0xff, 0x40, 0x0, 0x24, 0xed, 0x44, 0x10, 0x0,
    0x0, 0xce, 0x0, 0x0, 0x0, 0x0, 0xdb, 0x0,
    0x0, 0x0, 0x2, 0xf5, 0x0, 0x0, 0x0, 0xc,
    0xc1, 0x0, 0x0, 0x0, 0x9f, 0xff, 0xfd, 0xaa,
    0xd2, 0x5b, 0x64, 0x6a, 0xef, 0xd4, 0x0, 0x0,
    0x0, 0x0, 0x0,

    /* U+00B0 "°" */
    0x1a, 0xfb, 0x19, 0x80, 0x7b, 0xd1, 0x0, 0xfa,
    0x70, 0x5c, 0x1c, 0xfd, 0x20, 0x0, 0x0,

    /* U+00D7 "×" */
    0x6, 0x0, 0x0, 0x33, 0x5, 0xf9, 0x0, 0x2e,
    0xd0, 0x9, 0xf9, 0x2e, 0xe2, 0x0, 0x9, 0xff,
    0xe2, 0x0, 0x0, 0x1f, 0xf9, 0x0, 0x0, 0x1d,
    0xfc, 0xf6, 0x0, 0x1d, 0xf4, 0xb, 0xf6, 0x4,
    0xf4, 0x0, 0xc, 0xc0, 0x1, 0x0, 0x0, 0x0,
    0x0,

    /* U+00F7 "÷" */
    0x0, 0x0, 0xfd, 0x0, 0x0, 0x0, 0x0, 0xdc,
    0x0, 0x0, 0x28, 0x88, 0x88, 0x88, 0x81, 0x5f,
    0xff, 0xff, 0xff, 0xf3, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0xdc, 0x0, 0x0, 0x0, 0x0,
    0xfd, 0x0, 0x0,

    /* U+20AC "€" */
    0x0, 0x0, 0x7, 0xcf, 0xec, 0x40, 0x0, 0x3e,
    0xfc, 0x9a, 0xe9, 0x0, 0x1e, 0xf4, 0x0, 0x0,
    0x20, 0x8, 0xf6, 0x0, 0x0, 0x0, 0x0, 0xde,
    0x0, 0x0, 0x0, 0x1, 0xff, 0xff, 0xff, 0xff,
    0xb0, 0x4, 0xfb, 0x33, 0x33, 0x31, 0x0, 0x2f,
    0xa0, 0x0, 0x0, 0x0, 0x1f, 0xff, 0xff, 0xff,
    0xf3, 0x0, 0x3d, 0xf3, 0x33, 0x33, 0x0, 0x0,
    0x8f, 0x40, 0x0, 0x0, 0x0, 0x1, 0xee, 0x30,
    0x0, 0x1, 0x0, 0x4, 0xef, 0xc9, 0x9d, 0x60,
    0x0, 0x2, 0x8d, 0xff, 0xc3
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 160, .box_w = 10, .box_h = 15, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 75, .adv_w = 115, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 90, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 131, .adv_w = 158, .box_w = 10, .box_h = 7, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 166, .adv_w = 160, .box_w = 11, .box_h = 14, .ofs_x = -1, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0xd, 0x34, 0x54, 0x2009
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 163, .range_length = 8202, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 5, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 4,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t quiz_symbols_18 = {
#else
lv_font_t quiz_symbols_18 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 15,          /*The maximum line height required by the font*/
    .base_line = 1,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if QUIZ_SYMBOLS_18*/

