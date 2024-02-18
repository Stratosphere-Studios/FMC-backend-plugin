add_subdirectory(avionics)
target_sources(avionics_sys PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/fmc_sys.cpp
)
