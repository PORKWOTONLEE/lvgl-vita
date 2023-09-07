cmake_minimum_required(VERSION 3.6)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VITASDK})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
        set(CMAKE_PREFIX_PATH "$ENV{VITASDK}/arm-vita-eabi/lib/cmake")
        set(CMAKE_PREFIX_PATH "$ENV{VITASDK}/arm-vita-eabi/lib/cmake")
    else()
        message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
    endif()
endif()
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

project(lvgl_vita)

add_definitions(-DLV_CONF_PATH=${CMAKE_CURRENT_LIST_DIR}/platform/lv_conf.h)

if(DEBUG)
    add_definitions(-DLV_USE_PERF_MONITOR=1)
    add_definitions(-DLV_USE_MEM_MONITOR=1)
endif()

file(GLOB_RECURSE PLATFORM_SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/platform/*.c 
    ${CMAKE_CURRENT_LIST_DIR}/platform/*.o
)
file(GLOB_RECURSE LVGL_SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/*.c
)

add_library(${PROJECT_NAME} STATIC
    ${PLATFORM_SOURCES}
    ${LVGL_SOURCES}
)

target_compile_options(${PROJECT_NAME} PRIVATE -O3)

add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND bash ${CMAKE_CURRENT_LIST_DIR}/script/update_header.sh ${CMAKE_CURRENT_LIST_DIR}
)

if(USE_SAMPLE)
    add_subdirectory(sample)
endif()
