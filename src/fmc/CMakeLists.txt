FILE(GLOB FMC_SYS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
FILE(GLOB FMC_SYS_HDR "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_subdirectory(rad_nav)

add_library(fms_sys STATIC ${FMC_SYS_SRC} ${FMC_SYS_HDR})
target_include_directories(fms_sys INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
	TARGET_COMPILE_OPTIONS(fms_sys PRIVATE "/MT$<$<CONFIG:Release>:d>")
endif(WIN32)

add_xplane_sdk_definitions(fms_sys 400)

target_link_libraries(fms_sys PUBLIC radio_nav libnav libxp libtime)

if(UNIX AND NOT APPLE)
    set_property(TARGET fms_sys PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()
