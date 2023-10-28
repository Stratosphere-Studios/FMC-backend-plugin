FILE(GLOB RAD_NAV_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
FILE(GLOB RAD_NAV_HDR "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_library(radio_nav STATIC ${RAD_NAV_SRC} ${RAD_NAV_HDR})
target_include_directories(radio_nav INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
	TARGET_COMPILE_OPTIONS(radio_nav PRIVATE "/MT$<$<CONFIG:Release>:d>")
endif(WIN32)

add_xplane_sdk_definitions(radio_nav 400)

target_link_libraries(radio_nav PUBLIC libxp libnav libtime)

if(UNIX AND NOT APPLE)
    set_property(TARGET radio_nav PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()