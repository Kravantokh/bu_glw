# Benoe's Utilities: OpenGL Wrappers

A set of unopiniated C-style C++11 abstractions for OpenGL to make its usage less verbose and easier.

## Compilation
Compiling the project with the given `compile.sh` script requires:
 * CMake
 * Ninja
 
 ## Usage
 If you wish to incorporate this into your project add it as a git submodule and then use `add_subdirectory` in CMake to add it. Afterwards you may include the main header (`bu_glw.hpp`) into your project. *Do not* forget to include it only *after* including your OpenGL provider library or else the library will not compile due to the OpenGL functions no being declared.
 
