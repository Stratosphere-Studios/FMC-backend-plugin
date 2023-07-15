# FMC-backend-plugin

This plugin is meant to be the backend part of the fmc.
Please, note that this is not meant to be used outside of the
[Stratosphere 777 project](https://github.com/Stratosphere-Studios/777-300ER)

## Getting started

### Compiling on linux/mac:

1) Make sure you have CMake and g++ installed.
2) Make sure the default CMake compiler is g++. The plugin may not compile if you use something else.
3) Create a "build" directory inside src and cd into it.
4) Run the following command in the terminal:
```text
cmake .. && make
```

### Compiling on windows

1) Make sure you have CMake installed. You will also need the latest version of the msvc compiler.
2) Make sure the default CMake compiler is set to msvc.
3) Create a "build" directory inside src and cd into it.
4) Run the following commands in the terminal
```text
cmake ..
cmake --build "${build_directory_path}"
```
If the plugin has been compiled and integrated correctly, you should get files named Strato_777_apt.dat and Strato_777_rnw.dat in your X-Plane/Output/preferences directory.
