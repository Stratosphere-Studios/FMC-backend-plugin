FILE(GLOB LIBXP_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
FILE(GLOB LIBXP_HDR "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_library(libxp STATIC ${LIBXP_SRC} ${LIBXP_HDR})
target_include_directories(libxp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
	TARGET_COMPILE_OPTIONS(libxp PRIVATE "/MT$<$<CONFIG:Debug>:d>")
endif(WIN32)

add_xplane_sdk_definitions(libxp 400)

if(UNIX AND NOT APPLE)
    set_property(TARGET libxp PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()
