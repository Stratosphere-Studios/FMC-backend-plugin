FILE(GLOB LIBXP_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
FILE(GLOB LIBXP_HDR "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_library(libxp STATIC ${LIBXP_SRC} ${LIBXP_HDR})
target_include_directories(libxp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

#target_compile_options(libxp PUBLIC -Wall -Wextra -Werror -g)
IF(APPLE)
    TARGET_COMPILE_DEFINITIONS(libxp PUBLIC -DAPL=1 -DIBM=0 -DLIN=0)
ELSEIF(UNIX)
    TARGET_COMPILE_DEFINITIONS(libxp PUBLIC -DAPL=0 -DIBM=0 -DLIN=1)
ELSEIF(WIN32)
    TARGET_COMPILE_DEFINITIONS(libxp PUBLIC -DAPL=0 -DIBM=1 -DLIN=0)
ENDIF()
add_definitions(-DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1 -DXPLM400 -DDEBUG=1)

if(UNIX AND NOT APPLE)
    set_property(TARGET libxp PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()
