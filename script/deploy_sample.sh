#!/bin/bash

# copy header
cp -r ${1}/include/* ${1}/sample/inc

# copy lib 
cp ${1}/build/liblvgl_vita.a ${1}/sample/lib
