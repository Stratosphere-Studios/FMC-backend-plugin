# FMC-backend-plugin

This plugin is meant to be the backend part of the fmc.
Please, note that this is not meant to be used outside of the
[Stratosphere 777 project](https://github.com/Stratosphere-Studios/777-300ER)

## Getting started

This plugin uses [libacfutils](https://github.com/skiselkov/libacfutils) library. Make sure you have that installed before you proceed any further.

### Compiling on linux/mac:

1) Make sure you have CMake and g++ installed.
2) Make sure the default CMake compiler is g++. The plugin may not compile if you use something else.
3) Create a "build" directory inside the repository directory and cd into it.
4) Run the following command in the terminal:
```text
cmake .. -DLIBACFUTILS={Your libacfutils path}
make
```

### Compiling on windows

1) You will need to compile this plugin via minGW using g++. We don't support any other compilers to streamline the workflow across all platforms. 
2) You will need to install the following packages in the minGW: CMake, g++
3) Create a "build" directory inside the repository directory and cd into it.
4) Run the following commands in the terminal
```text
cmake .. -G "MSYS Makefiles" -DLIBACFUTILS={Your libacfutils path}
cmake --build .
```
If the plugin has been compiled and integrated correctly, you should get files named Strato_777_apt.dat and Strato_777_rnw.dat in your X-Plane/Output/preferences directory.


### Credits

[BRUHegg](https://github.com/BRUHegg): creator of this plugin
[Saso Kiselkov](https://github.com/skiselkov): creator of the libacfutils library
[Amy Parent](https://github.com/amyinorbit): mtcr-demo
