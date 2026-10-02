# PTCLTool

**PTCLTool** is a tool for editing the `.ptcl` particle effect format used in various Nintendo 3DS games, including:

- *Animal Crossing: New Leaf*
- *Mario Kart 7*
- *New Super Mario Bros. 2*
- *Photos with Mario*
- *Photos with Animal Crossing*
- *Super Mario 3D Land*
- *いっしょにフォト ピクミン*

**Note**: This editor is for the **sead PTCL format** it is **not compatible** with the later **NintendoWare EFT PTCL** format found in later 3DS titles.

## Features

- View and edit emitters within the PTCL file

## Requirements

- Qt 6.x, or **Qt 5.15** (required for **Windows 8.1 / 7**; Qt 6 needs Windows 10+)
- On Windows: **[win-iconv](https://github.com/win-iconv/win-iconv)**
- On Linux/macOS: Standard `iconv` library (usually pre-installed)

## Windows 8.1 build

Windows 8.1 is supported by building against **Qt 5.15** (Qt 6 requires Windows 10 or newer):

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=<path-to-Qt5.15.2>/msvc2019_64 -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
cmake --build build --config Release
windeployqt build\Release\PTCLTool.exe --release
```

A compiler with C++23 support is needed (Visual Studio 2022). `_WIN32_WINNT` is set to 0x0603 (Windows 8.1).

## Aknowledgements

This project includes the following third-party libraries:

- **[win-iconv](https://github.com/win-iconv/win-iconv)** – An iconv implementation for windows

- **[rg-etc1](https://github.com/richgel999/rg-etc1)** – Fast, single-file ETC1 texture compression  

Each library is included under its respective license in the `third_party/` directory.

- Icons are provided by **[icons8](https://icons8.com/)**


## Downloads

Prebuilt binaries for **Windows**, **macOS** and **Linux** can be found on the **[Latest Releases](https://github.com/ExplosBlue/PTCLTool/releases/latest)** page.

## License

PTCLTool is licensed under the GNU General Public License v3.0 (GPLv3).
See the LICENSE file for details.

You are free to use, modify, and redistribute this software under the terms of the GPLv3.
If you distribute binaries, you must also provide access to the corresponding source code.