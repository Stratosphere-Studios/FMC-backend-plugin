cmake_minimum_required(VERSION 3.13)
project(777_FMS CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wconversion \
    -Wsign-conversion -fPIC -g")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(avionics_sys "")


add_subdirectory(src/lib)
add_subdirectory(src/fmc)

add_definitions(-DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1 -DXPLM400 -DDEBUG=1)
if(APPLE)
	set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for Mac OS X" FORCE)
	add_definitions(-DAPL=1 -DIBM=0 -DLIN=0)
    set(ARCH "mac64")
elseif(WIN32)
	add_definitions(-DAPL=0 -DIBM=1 -DLIN=0)
    add_definitions(-D_WIN32_WINNT=0x0600)
    set(ARCH "mingw64")
    
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS} -static-libgcc -s")
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS}  -static-libgcc -static-libstdc++ -s")
else()
	add_definitions(-DAPL=0 -DIBM=0 -DLIN=1)
    set(ARCH "lin64")
endif()

# Set up libacfutils

option(LIBACFUTILS "")

set(SDK_ROOT "${LIBACFUTILS}/SDK")

# Set up our plugin

file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
FILE(GLOB HDR_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp")
add_library(stratosphere_fms_plugin SHARED ${SRC_FILES} ${HDR_FILES})

# Set up libnav

include(CMakePrintHelpers)
option(LIBNAV "/home/betatest/Documents/programming/libnav-redist" "")
cmake_print_variables(LIBNAV)
target_include_directories(avionics_sys PUBLIC "${LIBNAV}/include")
target_include_directories(stratosphere_fms_plugin PUBLIC "${LIBNAV}/include")
file(GLOB LIBNAV_LIBS ${LIBNAV}/${ARCH}/*.a)
cmake_print_variables(LIBNAV_LIBS)

# X-Plane Plugin stuff 
if(APPLE)
    find_library(XPLM_LIBRARY XPLM ${SDK_ROOT}/Libraries/Mac/)
    find_library(XPWIDGETS_LIBRARY XPWidgets ${SDK_ROOT}/Libraries/Mac/)
    find_library(CocoaLib Cocoa)
    
    target_link_libraries(stratosphere_fms_plugin PUBLIC ${XPLM_LIBRARY})
    target_link_libraries(stratosphere_fms_plugin PUBLIC ${XPWIDGETS_LIBRARY})
    target_link_libraries(stratosphere_fms_plugin PUBLIC ${CocoaLib})
elseif(WIN32)
    set(CMAKE_LIBRARY_PATH ${SDK_ROOT}/Libraries/Win/)
    find_library(XPLM_LIBRARY
        NAMES XPLM_64
        PATHS ${SDK_ROOT}/Libraries/Win/
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
    )
    find_library(XPWIDGETS_LIBRARY
        NAMES XPWidgets_64
        PATHS ${SDK_ROOT}/Libraries/Win/
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
    )
    target_link_libraries(libxp PUBLIC ${XPLM_LIBRARY} ${XPWIDGETS_LIBRARY})
    target_link_libraries(stratosphere_fms_plugin PUBLIC ${XPLM_LIBRARY} ${XPWIDGETS_LIBRARY})
endif()

#target_include_directories(stratosphere_fms_plugin PUBLIC ${SDK_ROOT}/CHeaders/XPLM)
target_include_directories(stratosphere_fms_plugin PUBLIC
    "${LIBACFUTILS}/libacfutils-redist/include"
    "${LIBACFUTILS}/libacfutils-redist/${ARCH}/include"
    "${SDK_ROOT}/CHeaders/XPLM"
    "${SDK_ROOT}/CHeaders/Widgets"
    "${SDK_ROOT}/CHeaders/Wrappers"
)
target_include_directories(libxp PUBLIC
    "${LIBACFUTILS}/libacfutils-redist/include"
    "${LIBACFUTILS}/libacfutils-redist/${ARCH}/include"
    "${SDK_ROOT}/CHeaders/XPLM"
    "${SDK_ROOT}/CHeaders/Widgets"
    "${SDK_ROOT}/CHeaders/Wrappers"
)
target_include_directories(stratosphere_fms_plugin PUBLIC "src/fmc")
target_include_directories(avionics_sys PUBLIC "src/lib/")


target_link_libraries(avionics_sys PUBLIC ${LIBNAV_LIBS} libxp libtime)
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
