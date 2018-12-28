cmake_minimum_required(VERSION 3.9)
project(SVG)

set(CMAKE_CXX_STANDARD 11)

set(ENV{PKG_CONFIG_PATH} "/opt/X11/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")

find_package(PkgConfig)

pkg_check_modules(FT pangoft2)
pkg_check_modules(PANGO pango)
pkg_check_modules(PANGOCAIRO pangocairo)

pkg_check_modules(CAIRO cairo)
pkg_check_modules(FC fontconfig)
pkg_check_modules(RSVG librsvg-2.0)

link_directories(/usr/local/opt/gettext/lib)
include_directories(/usr/local/opt/gettext/include)


project(shengyin VERSION 1.0.0 DESCRIPTION "A lib for create pngs.")
add_library(shengyin SHARED sources/shengyin.cpp)


target_link_libraries(shengyin ${FT_LIBRARIES})
target_link_libraries(shengyin ${FC_LIBRARIES})
target_link_libraries(shengyin ${PANGO_LIBRARIES})
target_link_libraries(shengyin ${RSVG_LIBRARIES})
target_link_libraries(shengyin ${PANGOCAIRO_LIBRARIES})
target_include_directories(shengyin PUBLIC ${PANGO_INCLUDE_DIRS})
target_include_directories(shengyin PUBLIC ${RSVG_INCLUDE_DIRS})
set_target_properties(shengyin PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(shengyin PROPERTIES PUBLIC_HEADER include/shengyin.h)
target_compile_options(shengyin PUBLIC ${RSVG_CPPFLAGS_OTHER})
target_include_directories(shengyin PRIVATE include)

add_executable(SVG main.cpp)
target_link_libraries(SVG shengyin)
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -v" )