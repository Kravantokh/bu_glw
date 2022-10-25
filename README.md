# Benoe's Utilities: OpenGL Wrappers

A set of unopiniated C-style C++11 abstractions for OpenGL to make its usage less verbose and easier.
This library also include gl3w as the OpenGL function loader.

## Cloning
Don't forget to clone the repository with:
```git clone --recursive https://github.com/Kravantokh/bu_glw```

## Compilation
Compiling the project with the given `compile.sh` script requires:
 * CMake
 * Ninja
 
 ## Usage
 If you wish to incorporate this into your project add it as a git submodule and then use `add_subdirectory` in CMake to add it. Afterwards you may include the main header (`bu_glw.hpp`) into your project.
For an example see my [OpenGL template](https://github.com/Kravantokh/OpenGL_template) lirary.
