# How to Set Up GLFW Library

[GLFW](https://www.glfw.org/) is a library to open a window with OpenGL graphics visualization. 

There are three ways to set up GLFW:  
1. build from the [source code on GitHub](https://github.com/glfw/glfw)
2. use package managers (apt-get, brew, vcpkg, etc)
3. download pre-build binaries

**This lecture only supports the first option (e.g., build from source code) because it is the most EDUCATIONAL.** Yet, you can choose the other options with your own responsibility. 

We will download the source code of GLFW library at `external/glfw`, build the library at `external/glfwbuild` and install it at `external/glfwlib`. First, take a look at the [compile manual of glfw](https://www.glfw.org/docs/3.3/compile.html).  Then, do the job with the following commands:

```bash
$ git submodule update --init external/glfw # download source code
$ cmake -S external/glfw -B external/glfwbuild # make build folder
$ cd external/glfwbuild 
$ cmake --build . --config Release # build inside build folder
$ cmake --install . --prefix ../glfwlib # install at glfwlib folder
```

Make sure you have a header file `glfw3.h` at
```
acg-<username>/external/glfwlib/include/GLFW/glfw3.h
```



## Linux

For Ubuntu, you may need to install dependencies before the installation. Use `apt-get` to install

```bash
$ sudo apt install libwayland-dev libxkbcommon-dev wayland-protocols extra-cmake-modules
```

for other unix-like OS, look at the [manual](https://www.glfw.org/docs/3.3/compile.html#compile_deps) to install dependencies. 



## Tips

* Why we make a folder (i.e., `glfwbuild`) just for building code? This is called ***out-of-source build*** and its a good practice for open source library.
* Do not commit library intermediate files or library itself. Make sure you only commit the codes/file you edited. When you see a lot of files by typing `git status` or `git add .` something is wrong.
* When things go out of your contrl, do it from scratch (e.g., `git clone https://github.com/ACG-2024S/acg-<username>.git`).