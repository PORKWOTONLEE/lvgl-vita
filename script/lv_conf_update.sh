#!/bin/bash

# copy lv_conf.h to lvgl projectl before compile
if [ ! -e ${1}/lv_conf.h ]
then
    cp ${1}/platform/lv_conf.h ${1}

    exit 0
fi

# remove lv_conf.h in lvgl projectl after compile
if [ -e ${1}/lv_conf.h ]
then
    rm ${1}/lv_conf.h
    cp ${1}/platform/lv_conf.h ${1}/include/src

    exit 0
fi

