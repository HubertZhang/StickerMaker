#include <iostream>
#include "include/shengyin.h"
#include <cmath>


using std::cout;
using std::endl;

cairo_status_t default_writer(void *closure, const unsigned char *data, unsigned int length) {
    static FILE* file = fopen("t7st.png", "wb");
    size_t size = fwrite(data, sizeof(unsigned char), length, file);
    if (size == length) {
        return CAIRO_STATUS_SUCCESS;
    }
    return CAIRO_STATUS_WRITE_ERROR;
}

cairo_status_t default_reader(void *closure, unsigned char *data, unsigned int length) {
    static FILE* file = fopen("t6st.png", "rb");

    size_t size = fread(data, sizeof(unsigned char), length, file);
    if (size == length) {
        return CAIRO_STATUS_SUCCESS;
    }
    return CAIRO_STATUS_WRITE_ERROR;
}

int main() {
//    generate_image("a啊a😎a啊g", "发出aaaaaaaaaaaaaaaa的声音", &default_writer);

    auto surface = sticker_init_from_svg_file("Chiba.svg");
    auto position = ImagePosition{100, 200, 100, 80, IMAGE_ANCHOR_RIGHT | IMAGE_ANCHOR_BOTTOM};
    std::cout << std::oct << position.imageAnchor << std::endl;
    sticker_add_image(surface, &default_reader, position);

    sticker_output_surface(surface, &default_writer);
    std::cout << "Hello, World!" << std::endl;
    return 0;
}