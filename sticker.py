import io
import sys
from ctypes import *
from typing import Dict

CAIRO_STATUS_SUCCESS = 0
CAIRO_STATUS_WRITE_ERROR = 11

WRITE_CALLBACK = CFUNCTYPE(c_int, *[c_void_p, POINTER(c_char), c_uint])

if sys.platform == "darwin":
    shengyin = cdll.LoadLibrary("./lib/libshengyin.1.0.0.dylib")
elif sys.platform == "linux":
    shengyin = cdll.LoadLibrary("./lib/libshengyin.so")


class TextPosition(Structure):
    _fields_ = [("anchor_x", c_int),
                ("anchor_y", c_int),
                ("constraint_x", c_int),
                ("constraint_y", c_int),
                ("expected_ratio", c_double)]


class FontConfig(Structure):
    _fields_ = [("font_size", c_int),
                ("bold", c_bool)]


shengyin.generate_image.argtypes = [c_char_p, c_char_p, WRITE_CALLBACK]

shengyin.sticker_set_default_description.argtypes = [c_char_p]

shengyin.sticker_init_from_png_file.argtypes = [c_char_p]
shengyin.sticker_init_from_png_file.restype = c_void_p

shengyin.sticker_init_from_svg_file.argtypes = [c_char_p]
shengyin.sticker_init_from_svg_file.restype = c_void_p

shengyin.sticker_copy_surface.argtypes = [c_void_p]
shengyin.sticker_copy_surface.restype = c_void_p

shengyin.sticker_init_from_svg_file.restype = c_void_p

shengyin.sticker_add_text.argtypes = [c_void_p, c_char_p, FontConfig, TextPosition]

shengyin.sticker_output_surface.argtypes = [c_void_p, WRITE_CALLBACK]


def create_callback_with_buffer(buffer: io.BytesIO):
    def write_callback(closure, data, length):
        try:
            buffer.write(data[0:length])
            return CAIRO_STATUS_SUCCESS
        except Exception:
            return CAIRO_STATUS_WRITE_ERROR

    return WRITE_CALLBACK(write_callback)


surface_buffer = dict()
shengyin.sticker_set_default_description("sans-serif,PingFangSC,\"Noto Sans CJK SC\" 24".encode("ascii"))


def generate_image(config, para: Dict[str, str]):
    source = config["source"]
    if surface_buffer.get(source) is None:
        if source.endswith("svg"):
            surface_buffer[source] = shengyin.sticker_init_from_svg_file(source.encode("utf-8"))
        elif source.endswith("png"):
            surface_buffer[source] = shengyin.sticker_init_from_png_file(source.encode("utf-8"))
    surface = shengyin.sticker_copy_surface(surface_buffer[source])
    font = FontConfig(0, False)
    if config.get("font") is not None:
        if config["font"].get("size") is not None:
            font.font_size = config["font"]["size"]
        if config["font"].get("bold") is not None:
            font.bold = True
    for k in config["texts"]:
        s = para.get(k)
        if s is None:
            continue
        t = TextPosition(*config["texts"][k])
        shengyin.sticker_add_text(surface, s.encode("utf-8"), font, t)

    storage = io.BytesIO()
    shengyin.sticker_output_surface(surface, create_callback_with_buffer(storage))
    storage.seek(0)
    return storage
