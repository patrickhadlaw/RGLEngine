# cpp-opengl

Simple OpenGL renderer, currently only draws triangle and transforms perspective camera on WASD keys and mouse movement.
Controls are:
* Click window to grab cursor.
* Press WASD, LCTRL, SPACE keys to move camera.
* Move mouse to rotate camera in relative x-plane and relative y-plane
* Press Q and E to rotate camera in relative z-plane

## Prerequisites

* CMake 3.0.0+
* OpenGL

## Liscensing

### This project is licensed under the provided license - see the [LICENSE](LICENSE) file for details

## Authors

* **Patrick Hadlaw** - [patrickhadlaw](https://github.com/patrickhadlaw)

## Build instructions

```
$ git clone https://github.com/patrickhadlaw/cpp-opengl
$ cd cpp-opengl
# NOTE: for Windows download the ZIP file from http://glew.sourceforge.net/ and extract as lib/glew, DO NOT clone the repository
$ git clone https://github.com/nigels-com/glew.git lib/glew
$ git clone https://github.com/glfw/glfw lib/glfw
$ git clone https://github.com/g-truc/glm lib/glm
$ git clone https://github.com/dtschump/CImg.git lib/cimg
# NOTE: freetype not required yet this line is just for reference
# $ git clone git://git.sv.nongnu.org/freetype/freetype2.git lib/freetype
$ mkdir bin
$ cd bin
$ cmake .. -G <CMake Generator>
# Build using generated files (ie run make for Unix Makefiles generator)
```

## Run instructions: 

```
$ cd <project-dir>/cpp-opengl/bin
$ ./cpp-opengl [window-width] [window-height]
```

![screenshot1](/screenshot1.PNG?raw=true "Screenshot")
