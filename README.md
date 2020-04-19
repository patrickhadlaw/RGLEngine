# RGLEngine

OpenGL real-time rendering engine.

Features:
* Render simple shapes
* Render images and textures
* Load vector font formats and render
* Physically based coordinate systems (ex. pt, ww, wh, vw, vh, ...)
* Physical quantity expressions, to allow simple operations (+, -, *, /) between different physical quantities (i.e. 3pt * 5cm + 10vw)
* Clickable UI elements (using triangle-raycast)
* Noclip camera transformation based on mouse and keyboard input
* Host-listener event system for capturing events such as window resize, keyboard key press, bounding box change, ...
* Sparse Voxel Octree renderer -> a raytracing algorithm runnable in real-time
* More to come...

Controls are:
* Click window to grab cursor.
* Press WASD, LCTRL, SPACE keys to move camera.
* Move mouse to rotate camera in relative x-plane and relative y-plane
* Press Q and E to rotate camera in relative z-plane
* Press Escape to ungrab cursor

## Prerequisites

* CMake 3.0.0+
* OpenGL 4.6

## Authors

* **Patrick Hadlaw** - [patrickhadlaw](https://github.com/patrickhadlaw)

## Build instructions

```
$ git clone https://github.com/patrickhadlaw/RGLEngine.git
$ cd RGLEngine
# NOTE: for Windows download the ZIP file from http://glew.sourceforge.net/ and extract as lib/glew, DO NOT clone the repository
$ git clone https://github.com/nigels-com/glew.git lib/glew
$ git clone https://github.com/glfw/glfw lib/glfw
$ git clone https://github.com/g-truc/glm lib/glm
$ git clone https://github.com/nothings/stb.git lib/stb
$ git clone git://git.sv.nongnu.org/freetype/freetype2.git lib/freetype
$ cd bin
$ cmake .. -G <CMake Generator>
# Build using generated files (ie run make for Unix Makefiles generator)
```

## Screenshots

### Interface
![interface](/screenshots/interface.png?raw=true "Interface")

### Sparse Voxel Octree
![sparse-voxel](/screenshots/sparse-voxel.png?raw=true "Sparse Voxel Octree")
