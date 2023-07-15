FILE(GLOB LIBNAV_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
FILE(GLOB LIBNAV_HDR "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_library(libnav STATIC ${LIBNAV_SRC} ${LIBNAV_HDR})
target_include_directories(libnav INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
	TARGET_COMPILE_OPTIONS(libnav PRIVATE "/MT$<$<CONFIG:Debug>:d>")
endif(WIN32)

add_xplane_sdk_definitions(libnav 400)

if(UNIX AND NOT APPLE)
    set_property(TARGET libnav PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()