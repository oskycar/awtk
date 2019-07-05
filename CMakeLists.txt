cmake_minimum_required(VERSION 3.12)
project(awtk)

set(CMAKE_CXX_STANDARD 11)

set(TK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(TK_SRC "${TK_ROOT}/src")

#----------------nanovg lib----------------#
set(nanovg_sources 
    "${TK_ROOT}/3rd/nanovg/base/nanovg.c")
add_library(
    nanovg
    STATIC
    ${nanovg_sources}
)
target_include_directories(
    nanovg
    PRIVATE ${TK_ROOT}/3rd/nanovg/base/
    PRIVATE ${TK_ROOT}/3rd/nanovg/gl/
)
#SET_SOURCE_FILES_PROPERTIES( ${nanovg_sources} PROPERTIES LANGUAGE CXX )

#----------------glad lib----------------#
set(glad_sources
    "${TK_ROOT}/3rd/glad/glad.c")
add_library(
    glad
    STATIC
    ${glad_sources}
)
target_include_directories(
    glad
    PRIVATE ${TK_ROOT}/3rd/glad/
)
#SET_SOURCE_FILES_PROPERTIES( ${glad_sources} PROPERTIES LANGUAGE CXX )

#----------------gpinyin lib----------------#
file(GLOB_RECURSE gpinyin_cpp_srcs LIST_DIRECTORIES true  "${TK_ROOT}/3rd/gpinyin/src/*.cpp")
add_library(
    gpinyin
    STATIC
    ${gpinyin_cpp_srcs}
)
target_include_directories(
    gpinyin
    PRIVATE ${TK_ROOT}/3rd/gpinyin/include/
    PRIVATE ${TK_ROOT}/3rd/gpinyin/src/
    PRIVATE ${TK_SRC}
)

#----------------linebreak lib----------------#
set(linebreak_sources
    "${TK_ROOT}/3rd/libunibreak/graphemebreak.c"
    "${TK_ROOT}/3rd/libunibreak/linebreak.c"
    "${TK_ROOT}/3rd/libunibreak/linebreakdef.c"
    "${TK_ROOT}/3rd/libunibreak/unibreakbase.c"
    "${TK_ROOT}/3rd/libunibreak/unibreakdef.c"
    "${TK_ROOT}/3rd/libunibreak/wordbreak.c"
    "${TK_ROOT}/3rd/libunibreak/emojidef.c"
    "${TK_ROOT}/3rd/libunibreak/linebreakdata.c")
add_library(
    linebreak
    STATIC
    ${linebreak_sources}
)
target_include_directories(
    linebreak
    PRIVATE ${TK_ROOT}/3rd/libunibreak/
)
#SET_SOURCE_FILES_PROPERTIES( ${linebreak_sources} PROPERTIES LANGUAGE CXX )


#----------------asseets lib----------------#
set(assets_sources
    "${TK_ROOT}/demos/assets.c")
add_library(
    assets
    STATIC
    ${assets_sources}
)
target_include_directories(
    assets
    PRIVATE "${TK_ROOT}/demos"
    PRIVATE "${TK_ROOT}/src"
    PRIVATE "${TK_ROOT}/src/ext_widgets"
)
#SET_SOURCE_FILES_PROPERTIES( ${assets_sources} PROPERTIES LANGUAGE CXX )


#----------------tools ----------------#

#----------------common lib ----------------#
set(common_sources
    "${TK_ROOT}/tools/common/utils.c")
add_library(
    common
    STATIC
    ${common_sources}
)
target_include_directories(
    common
    PRIVATE "${TK_ROOT}/tools/common/"
    PRIVATE "${TK_ROOT}/src"
)
#SET_SOURCE_FILES_PROPERTIES( ${common_sources} PROPERTIES LANGUAGE CXX )

#----------------font_gen lib ----------------#
set(font_gen_sources
    "${TK_ROOT}/tools/font_gen/font_gen.c")
add_library(
    font_gen
    STATIC
    ${font_gen_sources}
)
target_include_directories(
    font_gen
    PRIVATE "${TK_ROOT}/tools/"
    PRIVATE "${TK_ROOT}/tools/font_gen"
    PRIVATE "${TK_ROOT}/src"
)
#SET_SOURCE_FILES_PROPERTIES( ${font_gen_sources} PROPERTIES LANGUAGE CXX )

#----------------image_gen lib ----------------#
set(image_gen_sources
    "${TK_ROOT}/tools/image_gen/image_gen.c")
add_library(
    image_gen
    STATIC
    ${image_gen_sources}
)
target_include_directories(
    image_gen
    PRIVATE "${TK_ROOT}/tools"
    PRIVATE "${TK_ROOT}/tools/image_gen"
    PRIVATE "${TK_ROOT}/src"
)
#SET_SOURCE_FILES_PROPERTIES( ${image_gen_sources} PROPERTIES LANGUAGE CXX )

#----------------str_gen lib ----------------#
set(str_gen_sources
    "${TK_ROOT}/tools/str_gen/xml_str_gen.cc"
    "${TK_ROOT}/tools/str_gen/str_gen.cc")
add_library(
    str_gen
    STATIC
    ${str_gen_sources}
)
target_include_directories(
    str_gen
    PRIVATE "${TK_ROOT}/tools"
    PRIVATE "${TK_ROOT}/tools/str_gen/"
    PRIVATE "${TK_ROOT}/src"
)
#SET_SOURCE_FILES_PROPERTIES( ${str_gen_sources} PROPERTIES LANGUAGE CXX )

#----------------theme_gen lib ----------------#
set(theme_gen_sources
    "${TK_ROOT}/tools/theme_gen/theme_gen.cc"
    "${TK_ROOT}/tools/theme_gen/xml_theme_gen.cc")
add_library(
    theme_gen
    STATIC
    ${theme_gen_sources}
)
target_include_directories(
    theme_gen
    PRIVATE "${TK_ROOT}/tools"
    PRIVATE "${TK_ROOT}/tools/theme_gen/"
    PRIVATE "${TK_ROOT}/src"
)
#SET_SOURCE_FILES_PROPERTIES( ${theme_gen_sources} PROPERTIES LANGUAGE CXX )



#----------------awtk lib ----------------#

file(GLOB awtk_sources LIST_DIRECTORIES true 
    "${TK_SRC}/*.c"
    "${TK_SRC}/tkc/*.c"
    "${TK_SRC}/base/*.c"
    "${TK_SRC}/layouters/*.c"
    "${TK_SRC}/widgets/*.c"
    "${TK_SRC}/ui_loader/*.c"
    "${TK_SRC}/xml/*.c"
    "${TK_SRC}/svg/*.c"
    "${TK_SRC}/clip_board/*.c"
    "${TK_SRC}/font_loader/*.c"
    "${TK_SRC}/blend/*.c"
    "${TK_SRC}/ext_widgets/*.c"
    "${TK_SRC}/ext_widgets/*/*.c"
    "${TK_SRC}/image_loader/*.c"
    "${TK_SRC}/designer_support/*.c"
    "${TK_SRC}/widget_animators/*.c"
    "${TK_SRC}/misc/*.cpp"
    "${TK_SRC}/window_animators/*.c"
    "${TK_SRC}/dialog_highlighters/*.c"
    "${TK_SRC}/lcd/lcd_mem_*.c"
    "${TK_SRC}/platforms/pc/*.c"
    )
set(awtk_sources 
    ${awtk_sources} 
    "${TK_SRC}/main_loop/main_loop_simple.c"
    "${TK_SRC}/input_methods/input_method_creator.c"
    "${TK_SRC}/vgcanvas/vgcanvas_nanovg_gl.c"
    "${TK_SRC}/input_engines/input_engine_pinyin.cpp"
    "${TK_SRC}/lcd/lcd_nanovg.c"
    "${TK_SRC}/main_loop/main_loop_sdl_gpu.c"
    )

#配置sdl库
find_package(SDL2 REQUIRED)

add_library(
    awtk
    STATIC
    ${awtk_sources}
)
include_directories("${SDL2_INCLUDE_DIRS}/")
target_include_directories(
    awtk
    PRIVATE ${SDL2_INCLUDE_DIRS}
    PRIVATE "${TK_ROOT}/tools"
    PRIVATE "${TK_ROOT}/tools/theme_gen/"
    PRIVATE "${TK_ROOT}/src"
    PRIVATE "${TK_ROOT}/src/ext_widgets"
    PRIVATE "${TK_ROOT}/3rd"
    PRIVATE "${TK_ROOT}/3rd/libunibreak/"
    PRIVATE "${TK_ROOT}/3rd/nanovg/base/"
    PRIVATE "${TK_ROOT}/3rd/nanovg/gl/"
    PRIVATE "${TK_ROOT}/3rd/glad/"
    PRIVATE "${TK_ROOT}/3rd/gpinyin/include/"
)

add_definitions(-DRES_ROOT="${TK_ROOT}/demos"
    -DTK_ROOT="${TK_ROOT}"
    -DSTBTT_STATIC -DSTB_IMAGE_STATIC -DWITH_STB_IMAGE
    -DWITH_VGCANVAS -DWITH_UNICODE_BREAK -DWITH_DESKTOP_STYLE
    -DSDL2 -DHAS_STD_MALLOC -DWITH_SDL -DWITH_FS_RES -DHAS_STDIO
    -DWITH_NANOVG_GPU -DWITH_VGCANVAS_LCD
    -DWITH_NANOVG_GL3 -DWITH_NANOVG_GL 
    -DWITH_WIDGET_POOL=1000
    -D__APPLE__ -DHAS_PTHREAD -DMACOS -DENABLE_MEM_LEAK_CHECK1
    -D__STDC_LIMIT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_CONSTANT_MACROS  -DBGFX_CONFIG_RENDERER_METAL=1
    )




#build c demos
add_definitions(-DRES_ROOT="${TK_ROOT}/demos")


#配置APPLE平台库
#查找framework库，找到后会定义名称FRAMEWORK_${fwname}
macro(FIND_OSX_FRAMEWORK fwname)
    find_library(FRAMEWORK_${fwname}
            NAMES ${fwname}
            PATHS ${CMAKE_OSX_SYSROOT}/System/Library
            PATH_SUFFIXES Frameworks
            NO_DEFAULT_PATH)
    if( ${FRAMEWORK_${fwname}} STREQUAL FRAMEWORK_${fwname}-NOTFOUND)
        MESSAGE(ERROR ": Framework ${fwname} not found")
    else()
        MESSAGE(STATUS "Framework ${fwname} found at ${FRAMEWORK_${fwname}}")
    endif()
endmacro(FIND_OSX_FRAMEWORK)

if(APPLE)
    FIND_OSX_FRAMEWORK(Cocoa)
    FIND_OSX_FRAMEWORK(QuartzCore)
    FIND_OSX_FRAMEWORK(OpenGL)
    FIND_OSX_FRAMEWORK(Metal)
    FIND_OSX_FRAMEWORK(MetalKit)

    set(depend_apple_libs
            ${depend_apple_libs}
            ${FRAMEWORK_Cocoa}
            ${FRAMEWORK_QuartzCore}
            ${FRAMEWORK_OpenGL}
            ${FRAMEWORK_Metal}
            ${FRAMEWORK_MetalKit}
            )
    
endif(APPLE)

set(depends_includes 
         ${TK_ROOT}
         ${TK_ROOT}/src
         ${TK_ROOT}/3rd
         ${TK_ROOT}/src/ext_widgets
         ${TK_ROOT}/3rd/pixman
         ${TK_ROOT}/3rd/pixman/pixman
         ${TK_ROOT}/3rd/cairo/cairo
         ${TK_ROOT}/3rd/bgfx/bgfx/include
         ${TK_ROOT}/3rd/bgfx/bx/include
         ${TK_ROOT}/3rd/bgfx/bimg/include
         ${TK_ROOT}/3rd/agge
         ${TK_ROOT}/3rd/agg/include
         ${TK_ROOT}/3rd/nanovg
         ${TK_ROOT}/3rd/nanovg/gl
         ${TK_ROOT}/3rd/nanovg/base
         ${TK_ROOT}/3rd/nanovg/agge
         ${TK_ROOT}/3rd/nanovg/bgfx
         ${TK_ROOT}/3rd/SDL/src
         ${TK_ROOT}/3rd/SDL/include
         ${TK_ROOT}/3rd/agge/src
         ${TK_ROOT}/3rd/agge/include
         ${TK_ROOT}/3rd/gpinyin/include
         ${TK_ROOT}/3rd/libunibreak
         ${TK_ROOT}/tools
         ${TK_ROOT}/awtk-c-demos/demos/)

set(AWTK_LIB_PATH "${TK_ROOT}/cmakebuild")

set(awtk_libs 
    assets
    awtk
    common
    font_gen
    glad
    gpinyin
    image_gen
    linebreak
    nanovg
    str_gen
    theme_gen)

set(depend_compile_options
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>
    )

#遍历所有cpp文件，每一个cpp文件是一个独立的测试程序,需要排除demo_main.c文件，因为其他demo会inclue改文件
# file(GLOB awtk_demo_sources "${TK_ROOT}/awtk-c-demos/demos/[^demo_main]*.c")
set(awtk_demo_sources 
    "${TK_ROOT}/demos/demo1_app.c"
    "${TK_ROOT}/demos/demo_vg_app.c"
    "${TK_ROOT}/demos/demo_ui_app.c"
    "${TK_ROOT}/demos/demo_tr_app.c"
    "${TK_ROOT}/demos/demo_animator_app.c"
    "${TK_ROOT}/demos/demo_thread_app.c"
    "${TK_ROOT}/demos/demo_desktop.c"
    "${TK_ROOT}/demos/preview_ui.c"
)

file(GLOB awtk_demo_c_sources "${TK_ROOT}/../demo/awtk-c-demos/demos/[^demo_main]*.c")

set(awtk_demo_sources ${awtk_demo_sources} ${awtk_demo_c_sources})

#强制指定按照c++文件去编译，这样就不用去指定链接-lstdc++库了
SET_SOURCE_FILES_PROPERTIES( ${awtk_demo_sources} PROPERTIES LANGUAGE CXX )
SET_SOURCE_FILES_PROPERTIES( "${TK_ROOT}/demos/demo_main.c" PROPERTIES LANGUAGE CXX )
#递归为每个cpp文件指定一个可执行程序目标
foreach(source ${awtk_demo_sources})

    get_filename_component(name "${source}" NAME_WE)
    set(the_target "${name}")

    add_executable(
        ${the_target} 
        "${source}"        
        )

    target_include_directories(
        ${the_target} 
        PRIVATE ${SDL2_INCLUDE_DIRS}
        PRIVATE ${depends_includes}
        )
     target_compile_options(
        ${the_target}
        PRIVATE ${depend_compile_options}        
    )
    target_link_libraries(
        ${the_target} 
        ${depend_apple_libs}
        ${SDL2_LIBRARIES}
        ${awtk_libs})
endforeach()

