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

project(lvgl_vita-sample)

file(GLOB_RECURSE SAMPLE_SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/src/*.c
)

add_executable(${PROJECT_NAME}
    ${SAMPLE_SOURCES}
)

target_compile_options(${PROJECT_NAME} PRIVATE -O3)

target_link_directories(${PROJECT_NAME} PUBLIC lib)
target_link_libraries(${PROJECT_NAME} PUBLIC lvgl_vita;
    SceDisplay_stub;SceGxm_stub;SceTouch_stub;SceCtrl_stub;)

add_custom_target(pre_build ALL
    COMMAND bash ${CMAKE_CURRENT_LIST_DIR}/../script/deploy_sample.sh ${CMAKE_CURRENT_LIST_DIR}/..)
add_dependencies(${PROJECT_NAME} pre_build)
add_dependencies(pre_build lvgl_vita)

set(VITA_APP_NAME "lvgl_vita-sample")
set(VITA_TITLEID  "VSDK04580")
set(VITA_VERSION  "01.00")
vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE sce_sys/icon0.png sce_sys/icon0.png
  FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
  FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
  FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
)
