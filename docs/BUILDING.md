# Building DD-SSH

## Linux / Debian-based Mint

Required early packages:

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
  qt6-webengine-dev
```

## Configure and build

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Qt Creator

Open `CMakeLists.txt` in Qt Creator, configure the kit, build, and run.

## Qt WebEngine note

`dev 0.1.2.3` continues the first web terminal frontend work, so Qt WebEngine is now required for the default build. On Debian/Ubuntu/Mint systems this is usually provided by `qt6-webengine-dev`. If CMake reports that `Qt6WebEngineWidgetsConfig.cmake` is missing, install the Qt WebEngine development package and re-run CMake.
