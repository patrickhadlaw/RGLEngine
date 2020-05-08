# RGLEngine

Real-time OpenGL rendering engine.

## Example programs

### interface

Features:
* Render simple shapes
* Render images and textures
* Load vector font formats and render
* Physically based coordinate systems (ex. pt, ww, wh, vw, vh, ...)
* Physical quantity expressions, to allow simple operations (+, -, *, /) between different physical quantities (i.e. 3pt * 5cm + 10vw)
* Clickable UI elements (using triangle-raycast)
* Noclip camera transformation based on mouse and keyboard input
* Host-listener event system for capturing events such as window resize, keyboard key press, bounding box change, ...
* More to come...

### sparse-voxel
* Sparse Voxel Octree renderer -> a sparse raytracing algorithm runnable in real-time
* A noclip camera transformer -> transforms rays using a quaternion in the compute shader

## Noclip camera controls

Controls are:
* Click window to grab cursor.
* Press WASD, LCTRL, SPACE keys to move camera.
* Move mouse to rotate camera in relative x-plane and relative y-plane
* Press Q and E to rotate camera in relative z-plane
* Press Escape to ungrab cursor

## Prerequisites

* CMake 3.12+
* OpenGL 4.5

## Authors

* **Patrick Hadlaw** - [patrickhadlaw](https://github.com/patrickhadlaw)

## Build instructions

To build RGLEngine run the following in a bash shell:
```
$ git clone https://github.com/patrickhadlaw/RGLEngine.git
$ cd RGLEngine
$ ./lib/install.sh
$ mkdir build
$ cd build
$ cmake .. -G <CMake Generator>
$ cmake --build .
```

## Screenshots

### Interface
![interface](/screenshots/interface.png?raw=true "Interface")

### Sparse Voxel Octree
![sparse-voxel](/screenshots/sparse-voxel.png?raw=true "Sparse Voxel Octree")
