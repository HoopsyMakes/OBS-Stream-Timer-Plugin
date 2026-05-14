cmake_minimum_required(VERSION 3.16)

project(countdown-source LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(countdown-source MODULE
    src/plugin.cpp
)

set_target_properties(countdown-source PROPERTIES PREFIX "")

set(LIBOBS_INCLUDE_DIR "" CACHE PATH "Path to the OBS/libobs include directory")
set(LIBOBS_LIBRARY "" CACHE FILEPATH "Path to the OBS/libobs library or import library")
set(LIBOBS_DLL "" CACHE FILEPATH "Optional path to obs.dll when using a Windows import library")

function(add_manual_libobs_target)
    if(WIN32 AND LIBOBS_DLL)
        add_library(OBS::libobs SHARED IMPORTED)
        set_target_properties(OBS::libobs PROPERTIES
            IMPORTED_IMPLIB "${LIBOBS_LIBRARY}"
            IMPORTED_LOCATION "${LIBOBS_DLL}"
            INTERFACE_INCLUDE_DIRECTORIES "${LIBOBS_INCLUDE_DIR}"
        )
    else()
        add_library(OBS::libobs UNKNOWN IMPORTED)
        set_target_properties(OBS::libobs PROPERTIES
            IMPORTED_LOCATION "${LIBOBS_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${LIBOBS_INCLUDE_DIR}"
        )
    endif()
endfunction()

if(CMAKE_CROSSCOMPILING AND WIN32)
    message(STATUS "Configuring a Windows cross-build; provide Windows OBS headers and a mingw-compatible libobs import library")
endif()

if(LIBOBS_INCLUDE_DIR AND LIBOBS_LIBRARY)
    # Explicit paths are intentionally preferred over any libobs CMake package
    # found on the host. This is especially important when cross-compiling on
    # Linux, where a host libobs package may not describe a MinGW import library.
    add_manual_libobs_target()
    target_link_libraries(countdown-source PRIVATE OBS::libobs)
else()
    # Prefer the CMake package installed by OBS Studio. This is the path used by
    # current OBS builds on Windows and is also available on many Linux packages.
    if(DEFINED ENV{OBS_DIR})
        list(APPEND CMAKE_PREFIX_PATH
            "$ENV{OBS_DIR}"
            "$ENV{OBS_DIR}/cmake"
            "$ENV{OBS_DIR}/cmake/libobs"
            "$ENV{OBS_DIR}/lib/cmake/libobs"
            "$ENV{OBS_DIR}/libobs/cmake"
        )
    endif()

    find_package(libobs CONFIG QUIET)

    if(TARGET OBS::libobs)
        target_link_libraries(countdown-source PRIVATE OBS::libobs)
    elseif(TARGET libobs)
        target_link_libraries(countdown-source PRIVATE libobs)
    elseif(NOT WIN32)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(LIBOBS REQUIRED IMPORTED_TARGET libobs)
        target_link_libraries(countdown-source PRIVATE PkgConfig::LIBOBS)
    else()
        message(FATAL_ERROR
            "Unable to find libobs. Set OBS_DIR to your OBS install or build prefix, "
            "or pass -Dlibobs_DIR=/path/to/libobs/cmake, or pass both "
            "-DLIBOBS_INCLUDE_DIR=... and -DLIBOBS_LIBRARY=... "
            "(use a mingw-compatible import library such as libobs.dll.a when cross-building)."
        )
    endif()
endif()

if(WIN32)
    set(OBS_PLUGIN_DESTINATION "obs-plugins/64bit")
    set(OBS_DATA_DESTINATION "data/obs-plugins/countdown-source")
else()
    set(OBS_PLUGIN_DESTINATION "${CMAKE_INSTALL_LIBDIR}/obs-plugins")
    set(OBS_DATA_DESTINATION "${CMAKE_INSTALL_DATADIR}/obs/obs-plugins/countdown-source")
endif()

install(TARGETS countdown-source
    RUNTIME DESTINATION "${OBS_PLUGIN_DESTINATION}"
    LIBRARY DESTINATION "${OBS_PLUGIN_DESTINATION}"
)

install(DIRECTORY data/
    DESTINATION "${OBS_DATA_DESTINATION}"
)
