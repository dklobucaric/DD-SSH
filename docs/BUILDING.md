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
  qt6-tools-dev-tools
```

## Configure and build

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Qt Creator

Open `CMakeLists.txt` in Qt Creator, configure the kit, build, and run.
