# DD-SSH macOS Build Guide

**Checkpoint:** dev 0.1.6.6 — Andromeda  
**Phase:** macOS DMG/dependency polish

This guide documents the first native macOS build path for DD-SSH.

The initial validated target is an **Intel x86_64 macOS build** using:

```text
macOS 15.7.5
Xcode at /Applications/Xcode.app/Contents/Developer
Homebrew
CMake
Ninja
pkg-config
Qt 6.11.1 from the official Qt installer
Homebrew libssh / OpenSSL / zlib
```

This is not yet the final public macOS release process. It is the foundation for producing a local `.app` and an unsigned `.dmg` for tester distribution, with extra dependency-audit reporting added in `dev 0.1.6.5`.

---

## 1. Required tools

Verify the base toolchain:

```bash
sw_vers
uname -m
xcode-select -p
brew --version
cmake --version
ninja --version
pkg-config --version
```

Validated reference output:

```text
ProductVersion: 15.7.5
Architecture: x86_64
Xcode path: /Applications/Xcode.app/Contents/Developer
Homebrew 5.1.6
cmake 4.3.1
ninja 1.13.2
pkg-config 2.5.1
```

Install Homebrew dependencies if needed:

```bash
brew install cmake ninja pkg-config libssh openssl@3 zlib
```

Verify libssh:

```bash
brew --prefix libssh
pkg-config --modversion libssh
pkg-config --libs libssh
```

Expected libssh version during the first macOS pass:

```text
0.12.0
```

---

## 2. Required Qt components

The first macOS build was validated with Qt installed under:

```text
~/Qt/6.11.1/macos
```

Required Qt modules/components:

```text
Qt Widgets
Qt WebEngineWidgets
Qt WebChannel
Qt Positioning
macdeployqt
```

Verify them:

```bash
ls ~/Qt/6.11.1/macos/lib/cmake/Qt6WebEngineWidgets
ls ~/Qt/6.11.1/macos/lib/cmake/Qt6WebChannel
ls ~/Qt/6.11.1/macos/lib/cmake/Qt6Positioning
~/Qt/6.11.1/macos/bin/qmake -query QT_VERSION
```

If `Qt6Positioning` is missing, open Qt Maintenance Tool and add the Qt Positioning component for Qt 6.11.1/macOS.

---

## 3. Clone or update the repo

```bash
cd ~
git clone https://github.com/dklobucaric/DD-SSH.git
cd DD-SSH
```

If the repo already exists:

```bash
cd ~/DD-SSH
git pull origin main
```

Check the version:

```bash
grep "DD_SSH_VERSION_STRING" CMakeLists.txt
```

Expected for this checkpoint:

```text
set(DD_SSH_VERSION_STRING "dev 0.1.6.6")
```

---

## 4. Build using the helper script

```bash
cd ~/DD-SSH
./scripts/macos-build-release.sh
```

Default settings:

```text
QT_DIR=$HOME/Qt/6.11.1/macos
BUILD_DIR=build-macos-release
MACOS_ARCH=x86_64
CMAKE_BUILD_TYPE=Release
```

Override example:

```bash
QT_DIR="$HOME/Qt/6.11.1/macos" \
MACOS_ARCH=x86_64 \
./scripts/macos-build-release.sh
```

Expected output:

```text
build-macos-release/dd-ssh.app
```

---

## 5. Manual build command

Equivalent manual build:

```bash
cd ~/DD-SSH
rm -rf build-macos-release

export PKG_CONFIG_PATH="/usr/local/opt/libssh/lib/pkgconfig:/usr/local/opt/openssl@3/lib/pkgconfig:/usr/local/opt/zlib/lib/pkgconfig:$PKG_CONFIG_PATH"

cmake -S . -B build-macos-release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$HOME/Qt/6.11.1/macos" \
  -DCMAKE_OSX_ARCHITECTURES=x86_64

cmake --build build-macos-release
```

---

## 6. Run from the build folder

The macOS build creates an app bundle, not a plain executable at `build-macos-release/dd-ssh`.

Run it with:

```bash
open build-macos-release/dd-ssh.app
```

Or from the terminal:

```bash
./build-macos-release/dd-ssh.app/Contents/MacOS/dd-ssh
```

This build-folder app is for development/testing. It is not the final deployed app for other Macs.

---

## 7. Smoke test

After launching:

```text
[ ] Main window opens
[ ] About shows dev 0.1.6.6
[ ] Settings opens
[ ] Config path is created under the macOS user profile
[ ] Saved session can be created
[ ] SSH password/private-key auth works
[ ] xterm.js terminal opens
[ ] whoami works
[ ] htop or top works
```

---

## 8. Known macOS notes

- The first target is **Intel x86_64**.
- Apple Silicon support is planned later. The Intel build may run under Rosetta 2 on Apple Silicon Macs.
- The first package is unsigned and not notarized.
- macOS 12.x is not treated as a primary target for the Qt 6.11.1 build. Older macOS support may require a separate legacy Qt build experiment.
- Do not treat a build-folder `.app` as a portable release. Use `scripts/macos-deploy-release.sh` to create a deployed `.app` and `.dmg`.
