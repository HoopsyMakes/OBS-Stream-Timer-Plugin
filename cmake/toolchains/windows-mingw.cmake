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

set(CMAKE_C_COMPILER "${MINGW_TRIPLET}-gcc" CACHE FILEPATH "Windows cross C compiler")
set(CMAKE_CXX_COMPILER "${MINGW_TRIPLET}-g++" CACHE FILEPATH "Windows cross C++ compiler")
set(CMAKE_RC_COMPILER "${MINGW_TRIPLET}-windres" CACHE FILEPATH "Windows resource compiler")

# Keep CMake's compiler checks from trying to link or run Windows executables
# while configuring on Linux.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Keep host programs (cmake, pkg-config, etc.) discoverable from Linux while
# still allowing callers to pass OBS Windows SDK paths outside a sysroot.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
