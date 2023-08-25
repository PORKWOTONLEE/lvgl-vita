#!/bin/bash

# collect header files
cp ${1}/platform/lvgl_vita.h ${1}/include
cp ${1}/lvgl/lvgl.h          ${1}/include
cp -r ${1}/lvgl/src             ${1}/include

# remove useless files
rm `find ${1}/include -name *.c`
rm `find ${1}/include -name *.mk`

# update lvgl path
sed -i 's/\.\.\/lvgl\///g' ${1}/include/lvgl_vita.h
