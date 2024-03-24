FILE(GLOB LIBTIME_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
FILE(GLOB LIBTIME_HDR "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_library(libtime STATIC ${LIBTIME_SRC} ${LIBTIME_HDR})
target_include_directories(libtime INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(UNIX AND NOT APPLE)
    set_property(TARGET libtime PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()
#target_compile_options(libtime PUBLIC -Wall -Wextra -Werror -g)
