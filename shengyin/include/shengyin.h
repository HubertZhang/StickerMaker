//
// Created by Hubert Zhang on 2018/4/8.
//

#ifndef SHENGYIN_SHENGYIN_H
#define SHENGYIN_SHENGYIN_H

#include <cairo.h>

extern "C"
typedef struct {
    int anchor_x;
    int anchor_y;
    int constraint_x;
    int constraint_y;
    double expected_ratio;
} TextPosition;

extern "C"
typedef unsigned ImageAnchor;

const unsigned IMAGE_ANCHOR_LEFT = 0b1000;
const unsigned IMAGE_ANCHOR_RIGHT = 0b0100;
const unsigned IMAGE_ANCHOR_TOP = 0b0010;
const unsigned IMAGE_ANCHOR_BOTTOM = 0b0001;

extern "C"
typedef struct {
    int anchor_x;
    int anchor_y;
    int constraint_x;
    int constraint_y;
    ImageAnchor imageAnchor;
} ImagePosition;

typedef struct {
    int font_size;
    bool bold;
} FontConfig;

extern "C" void
sticker_set_default_description(const char *description);

extern "C" cairo_surface_t *
sticker_init_from_png_file(const char *filename);

extern "C" cairo_surface_t *
sticker_init_from_svg_file(const char *filename);

extern "C" cairo_surface_t *
sticker_copy_surface(cairo_surface_t *surface);

extern "C" void
sticker_add_text(cairo_surface_t *surface, const char *text, FontConfig fontConfig, TextPosition position);

extern "C" void
sticker_add_image(cairo_surface_t *surface, cairo_read_func_t func, ImagePosition imagePosition);

extern "C" void
sticker_output_surface(cairo_surface_t *surface, cairo_write_func_t func);

extern "C" void
generate_image(const char *sound, const char *des, cairo_write_func_t func);

#endif //SHENGYIN_SHENGYIN_H
