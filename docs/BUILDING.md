# Building DD-SSH

DD-SSH is a Qt 6 / C++ / CMake / libssh project.

## Linux dependencies

Debian/Ubuntu/Linux Mint style systems:

```bash
sudo apt update
sudo apt install -y \
  build-essential \
  cmake \
  ninja-build \
  git \
  pkg-config \
  gdb \
  qtcreator \
  qt6-base-dev \
  qt6-base-dev-tools \
  qt6-tools-dev \
  qt6-tools-dev-tools \
  qt6-webengine-dev \
  libssh-dev
```

If CMake reports missing Qt WebEngine:

```bash
sudo apt install qt6-webengine-dev
```

If CMake reports missing libssh:

```bash
sudo apt install libssh-dev
```

## Configure and build

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

Clean rebuild:

```bash
cmake --build build --clean-first
```

## Qt Creator

Open `CMakeLists.txt` in Qt Creator, configure a Qt 6 kit, then build and run.

## Current runtime requirements

- Qt Widgets
- Qt WebEngineWidgets
- Qt WebChannel
- libssh

xterm.js assets are bundled as Qt resources under:

```text
resources/xterm/
```

No CDN should be required for terminal rendering.

## Current tested platform

Linux remains the primary development platform. Windows native Debug and Release build/runtime validation is documented in `docs/WINDOWS_BUILD.md`; standalone deploy-folder validation is documented in `docs/WINDOWS_DEPLOYMENT.md`. macOS still needs dedicated validation.

## Version identity

The About dialog reads version strings from `CMakeLists.txt`:

```cmake
set(DD_SSH_VERSION_STRING "dev 0.1.5.7")
set(DD_SSH_CODENAME_STRING "Andromeda")
set(DD_SSH_MILESTONE_STRING "MF 0.2 candidate")
```

Every generated checkpoint should update the version string.


---

## Windows native build

A native Windows build has been validated during the Andromeda line using MSVC, Ninja, Qt 6.11.1 MSVC 2022 64-bit, Qt WebEngine/WebChannel/Positioning, vcpkg `libssh`, and vcpkg `pkgconf`.

See the dedicated guide:

- [Windows Build Guide](WINDOWS_BUILD.md)

Short version:

```cmd
cmake -S . -B build-win -G Ninja ^
  -DCMAKE_BUILD_TYPE=Debug ^
  -DCMAKE_PREFIX_PATH=C:\Qt\6.11.1\msvc2022_64 ^
  -DCMAKE_TOOLCHAIN_FILE=C:\dev\vcpkg\scripts\buildsystems\vcpkg.cmake ^
  -DPKG_CONFIG_EXECUTABLE=C:\dev\vcpkg\installed\x64-windows\tools\pkgconf\pkgconf.exe

cmake --build build-win
```

Release build testing should use `build-win-release` and `-DCMAKE_BUILD_TYPE=Release`.


## Icon resources

DD-SSH icon assets live under:

```text
resources/icons/
resources/windows/
resources/macos/
```

The Qt app icon is embedded as a Qt resource and loaded from:

```text
:/icons/dd-ssh.png
```

Windows executable icon integration uses:

```text
resources/windows/dd-ssh.rc
resources/windows/dd-ssh.ico
```

macOS bundle icon prep uses:

```text
resources/macos/dd-ssh.icns
resources/macos/dd-ssh.iconset/
```

Linux packaging can later install the PNG size variants from `resources/icons/` into the appropriate hicolor icon theme directories.


---

## Windows deployment

After a Release build succeeds, see [Windows Deployment Guide](WINDOWS_DEPLOYMENT.md) for the first `windeployqt`-based standalone deployment test and helper script:

```text
scripts/windows-deploy-release.bat
```
