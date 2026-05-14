cmake_minimum_required(VERSION 3.16)

project(countdown-source LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBOBS REQUIRED libobs)

add_library(countdown-source MODULE
    src/plugin.cpp
)

target_include_directories(countdown-source PRIVATE
    ${LIBOBS_INCLUDE_DIRS}
    /usr/include/obs
)

target_link_directories(countdown-source PRIVATE
    ${LIBOBS_LIBRARY_DIRS}
)

target_link_libraries(countdown-source PRIVATE
    ${LIBOBS_LIBRARIES}
)
