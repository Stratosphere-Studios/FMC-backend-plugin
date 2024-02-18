#Shout out to https://github.com/leecbaker. Some of his CMake code is used in this project.
project(777_FMS CXX)
cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -fPIC -g")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(avionics_sys "")


add_subdirectory(lib)
add_subdirectory(fmc)

add_xplane_sdk_definitions(avionics_sys 400)

target_link_libraries(avionics_sys PUBLIC libnav libxp libtime)

#target_compile_options(777_FMS PUBLIC -Wall -Wextra -Werror -g)
add_xplane_plugin(stratosphere_fms_plugin 400 777_FMS.cpp)

target_include_directories(stratosphere_fms_plugin PUBLIC "lib/3rd-party/xplane_sdk/sdk/CHeaders/XPLM")
target_include_directories(stratosphere_fms_plugin PUBLIC "fmc")
target_include_directories(avionics_sys PUBLIC "lib/")

target_link_libraries(stratosphere_fms_plugin PUBLIC avionics_sys)


set_target_properties(stratosphere_fms_plugin PROPERTIES PREFIX "")
set_target_properties(stratosphere_fms_plugin PROPERTIES SUFFIX "")
set_target_properties(stratosphere_fms_plugin PROPERTIES OUTPUT_NAME "777_systems.xpl")

set(PLUGIN_BUILD_DIR "${PROJECT_SOURCE_DIR}/777_systems")

if(WIN32)
	set_target_properties(stratosphere_fms_plugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_BUILD_DIR}/win_x64" )
	set_target_properties(stratosphere_fms_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_BUILD_DIR}/win_x64" )
elseif(APPLE)
	set_target_properties(stratosphere_fms_plugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_BUILD_DIR}/mac_x64" )
	set_target_properties(stratosphere_fms_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_BUILD_DIR}/mac_x64" )
else()
	set_target_properties(stratosphere_fms_plugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_BUILD_DIR}/lin_x64" )
	set_target_properties(stratosphere_fms_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_BUILD_DIR}/lin_x64" )
endif()
