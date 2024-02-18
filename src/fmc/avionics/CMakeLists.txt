target_sources(avionics_sys PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/rad_nav/navaid_selector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rad_nav/navaid_tuner.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rad_nav/radio.cpp
    ${CMAKE_CURRENT_LIST_DIR}/avionics.cpp
)

