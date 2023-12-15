cmake_minimum_required(VERSION 3.20.0)

set(BOARD nucleo_g474re)
# set(BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Zephyr)
project(nucleo_g474re)

target_sources(app PRIVATE src/main.c)

# Add drivers
add_subdirectory(drivers)