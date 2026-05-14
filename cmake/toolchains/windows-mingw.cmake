# Cross-compile this OBS plugin for 64-bit Windows from Linux with mingw-w64.
#
# Usage:
#   cmake -S . -B build-win \
#     -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/windows-mingw.cmake \
#     -DLIBOBS_INCLUDE_DIR=/path/to/windows/obs/libobs \
#     -DLIBOBS_LIBRARY=/path/to/windows/obs/libobs.dll.a

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(MINGW_TRIPLET "x86_64-w64-mingw32" CACHE STRING "mingw-w64 compiler triplet")
set(MINGW_BINDIR "" CACHE PATH "Directory containing the mingw-w64 tools, for example /usr/bin")

function(find_mingw_tool output_variable tool_name friendly_name required)
    if(MINGW_BINDIR)
        find_program(${output_variable}
            NAMES "${MINGW_TRIPLET}-${tool_name}"
            PATHS "${MINGW_BINDIR}"
            NO_DEFAULT_PATH
        )
    else()
        find_program(${output_variable} NAMES "${MINGW_TRIPLET}-${tool_name}")
    endif()

    if(NOT ${output_variable} AND required)
        message(FATAL_ERROR
            "Unable to find ${MINGW_TRIPLET}-${tool_name} for the Windows cross-build. "
            "Install mingw-w64, add the directory containing ${MINGW_TRIPLET}-${tool_name} "
            "to PATH, or pass -DMINGW_BINDIR=/path/to/mingw/bin. "
            "You can also pass -DCMAKE_${friendly_name}_COMPILER=/full/path/to/${MINGW_TRIPLET}-${tool_name}."
        )
    endif()
endfunction()

if(NOT CMAKE_CXX_COMPILER)
    find_mingw_tool(MINGW_CXX_COMPILER g++ CXX TRUE)
    set(CMAKE_CXX_COMPILER "${MINGW_CXX_COMPILER}" CACHE FILEPATH "Windows cross C++ compiler")
endif()

if(NOT CMAKE_C_COMPILER)
    find_mingw_tool(MINGW_C_COMPILER gcc C FALSE)
    if(MINGW_C_COMPILER)
        set(CMAKE_C_COMPILER "${MINGW_C_COMPILER}" CACHE FILEPATH "Windows cross C compiler")
    endif()
endif()

if(NOT CMAKE_RC_COMPILER)
    find_mingw_tool(MINGW_RC_COMPILER windres RC FALSE)
    if(MINGW_RC_COMPILER)
        set(CMAKE_RC_COMPILER "${MINGW_RC_COMPILER}" CACHE FILEPATH "Windows resource compiler")
    endif()
endif()

set(CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
    MINGW_TRIPLET
    MINGW_BINDIR
    CMAKE_C_COMPILER
    CMAKE_CXX_COMPILER
    CMAKE_RC_COMPILER
)

# Keep CMake's compiler checks from trying to link or run Windows executables
# while configuring on Linux.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Keep host programs (cmake, pkg-config, etc.) discoverable from Linux while
# still allowing callers to pass OBS Windows SDK paths outside a sysroot.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
