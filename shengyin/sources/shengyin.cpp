//
// Created by Hubert Zhang on 2018/4/8.
//

#include <cairo.h>
#include <librsvg/rsvg.h>
#include <pango/pango.h>
#include <pango/pangocairo.h>
#include <cmath>
#include <cstring>
#include <shengyin.h>


#define min(a, b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define max(a, b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

cairo_surface_t *shared_surface = nullptr;
PangoFontDescription *fontDescription = nullptr;

extern "C" void
sticker_set_default_description(const char *description) {
    fontDescription = pango_font_description_from_string(description);
}

extern "C" cairo_surface_t *
sticker_init_from_png_file(const char *filename) {
    cairo_surface_t *surface = cairo_image_surface_create_from_png(filename);
    return surface;
}

extern "C" cairo_surface_t *
sticker_init_from_svg_file(const char *filename) {
    GError *error = nullptr;
    auto handle = rsvg_handle_new_from_file(filename, &error);
    if (error != nullptr) {
        g_log("Init", G_LOG_FLAG_FATAL, "%s", error->message);
        return nullptr;
    }
    RsvgDimensionData dimensionData;
    rsvg_handle_get_dimensions(handle, &dimensionData);

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, dimensionData.width,
                                                          dimensionData.height);
    auto ctx = cairo_create(surface);
    rsvg_handle_render_cairo(handle, ctx);

    cairo_destroy(ctx);
    rsvg_handle_close(handle, &error);
    if (error != nullptr) {
        g_log("Init", G_LOG_FLAG_FATAL, "%s", error->message);
        cairo_surface_destroy(surface);
        return nullptr;
    }
    return surface;
}

extern "C" void
sticker_add_text(cairo_surface_t *surface, const char *text, FontConfig font_config, TextPosition position) {
    auto ctx = cairo_create(surface);
    auto layout = pango_cairo_create_layout(ctx);
    auto font = pango_font_description_copy(fontDescription);
    if (font_config.bold) {
        pango_font_description_set_weight(font, PANGO_WEIGHT_BOLD);
    }
    if (font_config.font_size > 0) {
        pango_font_description_set_absolute_size(font, pango_units_from_double(font_config.font_size));
    }

    pango_layout_set_font_description(layout, font);
    pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);

    pango_layout_set_text(layout, text, strlen(text));
    PangoRectangle rectangle;
    pango_layout_get_extents(layout, nullptr, &rectangle);

    double scale = 1;

    if (position.constraint_x > 0 && position.constraint_y > 0) {
        double width = pango_units_to_double(rectangle.width);
        double height = pango_units_to_double(rectangle.height);
        if (width > position.constraint_x || height > position.constraint_y) {
            if (position.expected_ratio > 0) {
                pango_layout_set_wrap(layout, PANGO_WRAP_CHAR);
                double max_ratio = position.expected_ratio * 1.5;
                double ratio = width / height;
                int t = 0;
                while (ratio > max_ratio && t < 5) {
                    double w = width / sqrt(ratio / max_ratio);
                    pango_layout_set_width(layout, pango_units_from_double(w));
                    pango_layout_get_extents(layout, nullptr, &rectangle);
                    width = pango_units_to_double(rectangle.width);
                    height = pango_units_to_double(rectangle.height);
                    ratio = width / height;
                    t += 1;
                }
            }
            scale = min(position.constraint_x / width, position.constraint_y / height);
        }
    } else if (position.constraint_x > 0) {
        double width = pango_units_to_double(rectangle.width);
        if (width > position.constraint_x) {
            scale = position.constraint_x / width;
        }
    } else if (position.constraint_y > 0) {
        double height = pango_units_to_double(rectangle.height);
        if (height > position.constraint_y) {
            scale = position.constraint_y / height;
        }
    }

    if (scale != 1) {
        auto matrix = pango_matrix_copy(pango_context_get_matrix(pango_layout_get_context(layout)));
        pango_matrix_scale(matrix, scale, scale);
        pango_context_set_matrix(pango_layout_get_context(layout), matrix);
        pango_layout_get_extents(layout, nullptr, &rectangle);
    }
    double width = pango_units_to_double(rectangle.width) * scale;
    double height = pango_units_to_double(rectangle.height) * scale;
    double xoffset = pango_units_to_double(rectangle.x) * scale;
    double yoffset = pango_units_to_double(rectangle.y) * scale;

    cairo_move_to(ctx, position.anchor_x - xoffset - width / 2.0, position.anchor_y - yoffset - height / 2.0);

    cairo_scale(ctx, scale, scale);
    pango_cairo_update_layout(ctx, layout);
    pango_cairo_show_layout(ctx, layout);

    pango_font_description_free(font);
    g_object_unref(layout);
    cairo_destroy(ctx);
}

extern "C" cairo_surface_t *
sticker_copy_surface(cairo_surface_t *surface) {
    auto test_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, cairo_image_surface_get_width(surface),
                                                   cairo_image_surface_get_height(surface));
    auto ctx = cairo_create(test_surface);
    cairo_set_source_surface(ctx, surface, 0, 0);
    cairo_paint(ctx);
    cairo_destroy(ctx);
    return test_surface;
}

extern "C" void
sticker_add_image(cairo_surface_t *surface, cairo_read_func_t func, ImagePosition imagePosition) {
    auto pics = cairo_image_surface_create_from_png_stream(func, nullptr);
    int height = cairo_image_surface_get_height(pics);
    int width = cairo_image_surface_get_width(pics);

    double scale = 1;
    if (imagePosition.constraint_x != 0 || imagePosition.constraint_y != 0) {
        double x_ratio = 1, y_ratio = 1;
        if (imagePosition.constraint_x != 0) {
            x_ratio = (imagePosition.constraint_x + 0.0) / width;
        }
        if (imagePosition.constraint_y != 0) {
            y_ratio = (imagePosition.constraint_y + 0.0) / height;
        }
        scale = min(x_ratio, y_ratio);
        height = height * scale;
        width = width * scale;
    }
    int ori_x = imagePosition.anchor_x;
    int ori_y = imagePosition.anchor_y;

    if (imagePosition.imageAnchor & IMAGE_ANCHOR_TOP) {

    } else if (imagePosition.imageAnchor & IMAGE_ANCHOR_BOTTOM) {
        ori_y -= height;
    } else {
        ori_y -= height / 2;
    }
    if (imagePosition.imageAnchor & IMAGE_ANCHOR_LEFT) {

    } else if (imagePosition.imageAnchor & IMAGE_ANCHOR_RIGHT) {
        ori_x -= width;
    } else {
        ori_x -= width / 2;
    }

    cairo_matrix_t matrix;
    cairo_matrix_init_identity(&matrix);
    cairo_matrix_translate(&matrix, -ori_x, -ori_y);
    cairo_matrix_scale(&matrix, 1/scale, 1/scale);

    auto pattern = cairo_pattern_create_for_surface(pics);
    cairo_pattern_set_matrix(pattern, &matrix);

    auto ctx = cairo_create(surface);
    cairo_set_source(ctx, pattern);
    cairo_paint(ctx);
    cairo_destroy(ctx);

    cairo_pattern_destroy(pattern);
    cairo_surface_destroy(pics);
}


extern "C" void
sticker_output_surface(cairo_surface_t *surface, cairo_write_func_t func) {
    cairo_surface_write_to_png_stream(surface, func, nullptr);
    cairo_surface_destroy(surface);
}

extern "C" void
generate_image(const char *sound, const char *des, cairo_write_func_t func) {

    if (!shared_surface) {
        shared_surface = sticker_init_from_svg_file("Chiba-none.svg");
    }
    if (!fontDescription) {
        fontDescription = pango_font_description_from_string("PingFang SC, Noto Color Emoji 24");
    }

    auto test_surface = sticker_copy_surface(shared_surface);

    FontConfig fc{0, false};
    TextPosition desPosition{150, 275, 288, 0, 0};
    sticker_add_text(test_surface, des, fc, desPosition);

    TextPosition soundPosition{62, 88, 88, 65, 1.5};
    sticker_add_text(test_surface, sound, fc, soundPosition);

    cairo_surface_write_to_png_stream(test_surface, func, nullptr);
    cairo_surface_destroy(test_surface);
}
