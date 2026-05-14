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

### Windows on Windows

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

### Windows from Linux

Install a mingw-w64 toolchain and use the included CMake toolchain file. The
resulting DLL is a Windows plugin; Linux can build it but cannot run it.

On Ubuntu or Debian, the compiler package is typically installed with:

```sh
sudo apt-get install g++-mingw-w64-x86-64 binutils-mingw-w64-x86-64
```

CMake must be able to find `x86_64-w64-mingw32-g++`. That means the directory
containing that executable must be on `PATH`. For distro packages this is
usually already `/usr/bin`; verify it with:

```sh
command -v x86_64-w64-mingw32-g++
```

If the command prints nothing, add the MinGW `bin` directory to `PATH` before
configuring. For example, if the compiler is in `/opt/mingw64/bin`:

```sh
export PATH="/opt/mingw64/bin:$PATH"
```

Alternatively, pass the directory directly to CMake with
`-DMINGW_BINDIR=/opt/mingw64/bin`, or pass the compiler explicitly with
`-DCMAKE_CXX_COMPILER=/opt/mingw64/bin/x86_64-w64-mingw32-g++`.

Then configure and build. Start from a clean Windows build directory whenever
you change generators, toolchains, or cached OBS paths:

```sh
rm -rf build-win

cmake -S . -B build-win \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/windows-mingw.cmake \
  -DLIBOBS_INCLUDE_DIR="$HOME/vendor/obs-src/libobs" \
  -DLIBOBS_LIBRARY="$HOME/vendor/obs-mingw-lib/libobs.dll.a" \
  -DLIBOBS_DLL="$OBS_DLL"
cmake --build build-win
```

`LIBOBS_LIBRARY` must point at a MinGW-compatible Windows import library, such
as `libobs.dll.a`. MSVC `.lib` files from a Visual Studio OBS build are not
reliably linkable with mingw-w64. If you also have the matching `obs.dll`, you
can pass it with `-DLIBOBS_DLL=/path/to/obs.dll` so CMake records both the DLL
and import library on the imported target.

If your Windows OBS build provides a MinGW-compatible `libobsConfig.cmake`, you
can point `OBS_DIR` or `libobs_DIR` at that package instead of passing explicit
`LIBOBS_*` paths.

## Troubleshooting

### `Ninja does not match the generator used previously`

CMake stores the generator in the build directory cache. If `build-win` was
first configured with Unix Makefiles, you cannot reuse that same directory with
`-G Ninja`. Delete it and configure again, or use a different build directory:

```sh
rm -rf build-win
# or pick a fresh directory, such as: cmake -S . -B build-win-ninja -G Ninja ...
```

### `target pattern contains no '%'` after a failed configure

Do not keep building a directory after CMake reports a configure error. Clean the
build directory and configure again. Also avoid quoting `~` in CMake paths:
`"~/vendor/..."` is passed literally to CMake by the shell. Use `$HOME` or an
absolute path instead, for example `"$HOME/vendor/obs-src/libobs"`.

### `IMPORTED_IMPLIB not set for imported target "OBS::libobs"`

This happens when CMake finds a `libobs` package target that does not describe a
MinGW import library for the Windows cross-build. Pass both manual paths and this
project will prefer them over the discovered package:

```sh
rm -rf build-win

cmake -S . -B build-win \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/windows-mingw.cmake \
  -DLIBOBS_INCLUDE_DIR="$HOME/vendor/obs-src/libobs" \
  -DLIBOBS_LIBRARY="$HOME/vendor/obs-mingw-lib/libobs.dll.a" \
  -DLIBOBS_DLL="$OBS_DLL"
```

If you still see `Manually-specified variables were not used by the project`, you
are configuring an older checkout or an old cached build directory. Pull the
latest changes and remove `build-win` before configuring again.
