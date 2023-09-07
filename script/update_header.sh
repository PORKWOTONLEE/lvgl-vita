#!/bin/bash

# collect header
cp ${1}/platform/lvgl_vita.h ${1}/include
cp ${1}/lvgl/lvgl.h          ${1}/include/lvgl
cp -r ${1}/lvgl/src          ${1}/include/lvgl

# remove useless files
rm `find ${1}/include/lvgl -name *.c`
rm `find ${1}/include/lvgl -name *.mk`

# update lvgl path
sed -i 's/\.\.\///g' ${1}/include/lvgl_vita.h
