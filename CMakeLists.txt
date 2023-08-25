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

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

project(lvgl_vita)

option(USE_SAMPLE "build sample" OFF)

file(GLOB_RECURSE PORT_SOURCES   platform/*.c platform/*.o)
file(GLOB_RECURSE LVGL_SOURCES   lvgl/src/*.c)

add_library(${PROJECT_NAME} STATIC
    ${PORT_SOURCES}
    ${LVGL_SOURCES}
)

target_compile_options(${PROJECT_NAME} PRIVATE -O3)

add_custom_target(pre_build
    COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/script/lv_conf_update.sh ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Pre Build Jobs"
)
add_dependencies(${PROJECT_NAME} pre_build)

add_custom_target(post_build ALL
    COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/script/lv_conf_update.sh ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/script/header_files_update.sh ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Post Build Jobs"
)
add_dependencies(post_build ${PROJECT_NAME})

if(USE_SAMPLE)
    add_subdirectory(sample)
endif()
