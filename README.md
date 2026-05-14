# OBS Stream Timer Plugin

Native OBS source plugin that wraps the OBS browser source and points it at the
OBS Stream Timer countdown page.

## Build

### Linux

Install OBS Studio development files for your distribution, then configure and
build with CMake:

```sh
cmake -S . -B build
cmake --build build
```

### Windows

Install or build OBS Studio for Windows first. Then configure this plugin with a
Visual Studio generator and point CMake at the OBS installation or build prefix.
For a default 64-bit OBS install, for example:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
  -Dlibobs_DIR="C:/Program Files/obs-studio/cmake/libobs"
cmake --build build --config Release
```

If your OBS package does not provide `libobsConfig.cmake`, pass the include and
library paths explicitly instead:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
  -DLIBOBS_INCLUDE_DIR="C:/path/to/obs-studio/libobs" `
  -DLIBOBS_LIBRARY="C:/path/to/obs-studio/build/libobs/Release/obs.lib"
cmake --build build --config Release
```

You can also set `OBS_DIR` to the OBS install or build prefix before running
CMake, and the build will search that prefix for the OBS CMake package.
