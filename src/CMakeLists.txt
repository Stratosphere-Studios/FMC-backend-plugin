#Shout out to https://github.com/leecbaker. Some of his CMake code is used in this project.
project(777_FMS CXX)
cmake_minimum_required(VERSION 2.8)

add_subdirectory(lib)
add_subdirectory(fmc)


add_xplane_plugin(stratosphere_fms_plugin 400 777_FMS.cpp)

target_link_libraries(stratosphere_fms_plugin PUBLIC fmc_sys)

SET_PROPERTY(TARGET stratosphere_fms_plugin PROPERTY CXX_STANDARD 20)
