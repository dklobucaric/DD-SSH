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

Linux is the primary tested platform so far.

Windows/macOS are project targets but need dedicated validation.

## Version identity

The About dialog reads version strings from `CMakeLists.txt`:

```cmake
set(DD_SSH_VERSION_STRING "dev 0.1.4.9")
set(DD_SSH_CODENAME_STRING "Andromeda")
set(DD_SSH_MILESTONE_STRING "MF 0.2 candidate")
```

Every generated checkpoint should update the version string.
