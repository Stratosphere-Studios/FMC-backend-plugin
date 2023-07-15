FILE(GLOB FMC_SYS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
FILE(GLOB FMC_SYS_HDR "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_library(fmc_sys STATIC ${FMC_SYS_SRC} ${FMC_SYS_HDR})
target_include_directories(fmc_sys INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
	TARGET_COMPILE_OPTIONS(fmc_sys PRIVATE "/MT$<$<CONFIG:Debug>:d>")
endif(WIN32)

add_xplane_sdk_definitions(fmc_sys 400)

target_link_libraries(fmc_sys PUBLIC libxp libnav)

if(UNIX AND NOT APPLE)
    set_property(TARGET fmc_sys PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()
