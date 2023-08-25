cmake_minimum_required(VERSION 3.6)

# vitasdk cross compile config
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

# lvgl require
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# project detail
set(VITA_APP_NAME "lvgl_vita-demo")
set(VITA_TITLEID  "VSDK04580")
set(VITA_VERSION  "01.00")
project(lvgl_vita-demo)

file(GLOB_RECURSE PROJECT_SOURCES  ../lvgl/demos/widgets/*.c main.c)
file(GLOB_RECURSE PLATFORM_SOURCES ../platform/*.c ../platform/shader/compiled/*.o)
file(GLOB_RECURSE LVGL_SOURCES     ../lvgl/src/*.c)

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${PLATFORM_SOURCES}
    ${LVGL_SOURCES}
)

target_compile_options(${PROJECT_NAME} PRIVATE -O3)

target_link_libraries(${PROJECT_NAME} PUBLIC 
    SceDisplay_stub;SceGxm_stub;SceTouch_stub;SceCtrl_stub;)

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE sce_sys/icon0.png sce_sys/icon0.png
  FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
  FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
  FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
)
