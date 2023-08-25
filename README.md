# LVGL-Vita
![](https://github.com/PORKWOTONLEE/lvgl-vita/blob/master/pic/screenshot.png)

LVGL-Vita is Light and Versatile Graphics Library that port for PSVITA

More about LVGL:&nbsp;&nbsp;&nbsp;&nbsp;[[Github]](https://github.com/lvgl/lvgl)&nbsp;&nbsp;&nbsp;&nbsp;[[Online Document]](https://docs.lvgl.io/master/index.html)

## Works & WIP
Works
- Display (with single/double frame buffer, and fully/partial screen fresh)
- Front touch
- Button

WIP
- Improve display FPS, reduce tear
- Optimized GPU driver
- Add back touch (maybe, front touch is enough)

## Enviroment
- **WSL** (Distro: Ubuntu)
- **VitaSDK** (Better download & configure by vdpm)

## Project Structure
- **include**: header file here
- **build**: lib here
- **lvgl**: LVGL source code
- **platform**: PSVITA porting middleware
- **sample**: just sample

## Build Instruction
Build **static library**
```shell
git clone --recursive git@github.com:PORKWOTONLEE/lvgl-vita.git
cd lvgl-vita && cd build
cmake ..
make

```
Build **sample**
```shell
git clone --recursive git@github.com:PORKWOTONLEE/lvgl-vita.git
cd lvgl-vita && cd sample && cd build
cmake ..
make

```
cmake with -DUSE_SAMPLE=ON to build both of lib & sample

## Usage
All you need is:

- Copy the whole include folder to your project and include the header file: **lvgl_vita.h**

- Link the static library: **liblvgl_vita.a** and the essential libraries libSceDisplay_stub.a, libSceGxm_stub.a, libSceTouch_stub.a, libSceCtrl_stub.a

- Finally code likes this:
```c
#include "lvgl_vita.h"

int main(void)
{
	psv_lv_init();

	// write your code here before mainloop

	psv_lv_mainloop();

	psv_lv_deinit();
}
```

