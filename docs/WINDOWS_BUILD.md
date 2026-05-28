# Windows Build Guide

**Checkpoint:** dev 0.1.5.9 — Andromeda
**Purpose:** document the native Windows build path, confirmed Release build procedure, and handoff to the standalone deployment test.

This guide documents the Windows setup that was validated during the Andromeda line. It is intentionally practical and conservative: first get a native Windows build running, then test runtime behavior, then later experiment with deployment/installer packaging.

---

## Current Windows status

Confirmed during the first native Windows test pass:

```text
[x] MSVC x64 compiler works
[x] CMake configure works
[x] Ninja build works
[x] Qt app launches on Windows
[x] Qt app theme follows Windows dark mode when set to System
[x] Settings and Welcome screen open
[x] SSH connection works
[x] xterm.js local renderer works through Qt WebEngine
[x] htop runs inside the Windows-built DD-SSH terminal
[x] Release build succeeds with build-win-release
```

Known Windows observations from the first Windows build tests:

```text
- app startup is slower than Linux
- first xterm.js terminal can take several seconds because Qt WebEngine/Chromium starts lazily
- later terminal tabs open much faster
- Task Manager showed roughly 350–380 MB RAM with a WebEngine terminal open
- Qt may create a cache folder under the DD-SSH AppData location
```

These are not currently treated as fatal bugs. They are expected side effects of using Qt WebEngine/xterm.js. Release builds should be used for standalone deployment testing.

---

## Recommended Windows approach

Use a **native Windows build**, not WSL, for Windows validation.

Recommended stack:

```text
Windows 10/11
Visual Studio 2022 Build Tools / MSVC x64
CMake
Ninja
Git for Windows
Qt 6 MSVC 2022 64-bit
Qt WebEngine
Qt WebChannel
Qt Positioning
vcpkg
libssh from vcpkg
pkgconf from vcpkg
```

Do not start with MinGW for this project. The first tested path uses MSVC.

---

## 1. Install Visual Studio C++ Build Tools

Install Visual Studio 2022 Build Tools or Visual Studio Community 2022.

Required workload:

```text
Desktop development with C++
```

Required components:

```text
MSVC v143/v144 x64 compiler toolset
Windows 10/11 SDK
C++ CMake tools for Windows
```

Open:

```text
x64 Native Tools Command Prompt for VS 2022
```

Verify:

```cmd
cl
cmake --version
ninja --version
git --version
```

Expected: `cl` prints Microsoft C/C++ compiler information, not `'cl' is not recognized`.

---

## 2. Install Qt

Use Qt Maintenance Tool / Online Installer.

Tested path:

```text
C:\Qt\6.11.1\msvc2022_64
```

Required Qt components:

```text
Qt 6.11.1 MSVC 2022 64-bit
Qt WebEngine
Qt WebChannel
Qt Positioning
```

Verify:

```cmd
dir C:\Qt\6.11.1\msvc2022_64\lib\cmake\Qt6WebEngineCore
dir C:\Qt\6.11.1\msvc2022_64\lib\cmake\Qt6WebEngineWidgets
dir C:\Qt\6.11.1\msvc2022_64\lib\cmake\Qt6WebChannel
dir C:\Qt\6.11.1\msvc2022_64\lib\cmake\Qt6Positioning
```

If CMake says WebEngine is missing because `Qt6Positioning` could not be found, install Qt Positioning through the Maintenance Tool.

---

## 3. Install vcpkg, libssh, and pkgconf

From the x64 Native Tools prompt:

```cmd
cd C:\
mkdir dev
cd C:\dev

git clone https://github.com/microsoft/vcpkg.git
cd C:\dev\vcpkg

bootstrap-vcpkg.bat
vcpkg install libssh:x64-windows
vcpkg install pkgconf:x64-windows
```

Verify pkgconf path:

```cmd
dir /s /b C:\dev\vcpkg\*pkgconf.exe
```

Expected path:

```text
C:\dev\vcpkg\installed\x64-windows\tools\pkgconf\pkgconf.exe
```

Current CMake uses `pkg-config`/`pkgconf` for libssh discovery. A future CMake polish may prefer `find_package(libssh CONFIG REQUIRED)` on Windows, but the documented working setup uses pkgconf.

---

## 4. Clone DD-SSH

```cmd
cd C:\dev
git clone https://github.com/dklobucaric/DD-SSH.git
cd C:\dev\DD-SSH
```

For Windows experiments:

```cmd
git checkout feature/windows-build
```

For testing the main development line:

```cmd
git fetch origin
git checkout -b dev origin/dev
```

---

## 5. Debug build

Clean old build folder:

```cmd
cd C:\dev\DD-SSH
rmdir /s /q build-win
```

Configure:

```cmd
cmake -S . -B build-win -G Ninja ^
  -DCMAKE_BUILD_TYPE=Debug ^
  -DCMAKE_PREFIX_PATH=C:\Qt\6.11.1\msvc2022_64 ^
  -DCMAKE_TOOLCHAIN_FILE=C:\dev\vcpkg\scripts\buildsystems\vcpkg.cmake ^
  -DPKG_CONFIG_EXECUTABLE=C:\dev\vcpkg\installed\x64-windows\tools\pkgconf\pkgconf.exe
```

Build:

```cmd
cmake --build build-win
```

Run from the same terminal:

```cmd
set PATH=C:\Qt\6.11.1\msvc2022_64\bin;C:\dev\vcpkg\installed\x64-windows\bin;%PATH%
build-win\dd-ssh.exe
```

---

## 6. Release build test

The Release build is the first meaningful performance test. Debug builds are expected to be slower and heavier.

```cmd
cd C:\dev\DD-SSH
rmdir /s /q build-win-release

cmake -S . -B build-win-release -G Ninja ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_PREFIX_PATH=C:\Qt\6.11.1\msvc2022_64 ^
  -DCMAKE_TOOLCHAIN_FILE=C:\dev\vcpkg\scripts\buildsystems\vcpkg.cmake ^
  -DPKG_CONFIG_EXECUTABLE=C:\dev\vcpkg\installed\x64-windows\tools\pkgconf\pkgconf.exe

cmake --build build-win-release
```

Run:

```cmd
set PATH=C:\Qt\6.11.1\msvc2022_64\bin;C:\dev\vcpkg\installed\x64-windows\bin;%PATH%
build-win-release\dd-ssh.exe
```

Measure and record:

```text
- app cold start time
- first xterm.js terminal open time
- second xterm.js terminal open time
- RAM after Welcome screen only
- RAM after one xterm terminal
- RAM after htop is running
```

Suggested notes format:

```text
Build type: Release
Windows version:
Qt version:
CPU/RAM:
Welcome startup:
First terminal open:
Second terminal open:
RAM Welcome:
RAM with terminal:
Notes:
```

---

## 7. First runtime tests on Windows

Before testing SSH, verify UI basics:

```text
Help → About DD-SSH
Tools → Settings
File → Open Config Folder
File → Export Config
```

Expected config path style:

```text
C:\Users\<user>\AppData\Local\DD-LAB\DD-SSH\dd-ssh.json
```

Then test SSH:

```text
Session → New Session
```

Suggested first test:

```text
password auth → save session → double-click saved session → xterm.js terminal opens
```

Terminal commands:

```bash
whoami
hostname
stty size
htop
exit
```

Also test:

```text
Disconnect
Reconnect
Settings → App theme System/Light/Dark
Settings → font size applies to newly opened terminals
```

---

## 8. Expected warnings and known quirks

### WrapVulkanHeaders warning

CMake may print:

```text
Could NOT find WrapVulkanHeaders (missing: Vulkan_INCLUDE_DIR)
```

If configure still reaches:

```text
Configuring done
Generating done
```

then this warning is currently non-fatal for DD-SSH.

### First terminal startup delay

The first xterm terminal on Windows may take several seconds because Qt WebEngine initializes Chromium/WebEngine resources lazily. Later terminal tabs are much faster.

Future polish may add:

```text
- clearer “Starting terminal engine...” message
- optional WebEngine preload setting
```

### RAM usage

Qt WebEngine embeds a Chromium-based engine. With one active xterm/WebEngine terminal, RAM usage can be hundreds of MB. This is expected for the current terminal architecture and should be documented for public alpha testers.

### Qt cache folder

Qt may create cache folders under:

```text
C:\Users\<user>\AppData\Local\DD-LAB\DD-SSH\cache\
```

This is runtime/cache data, not DD-SSH session/secrets config.

---

## 9. Deployment handoff

This guide validates building and running from the Windows build environment. Standalone deploy-folder testing is tracked separately in `docs/WINDOWS_DEPLOYMENT.md`.

Current deployment checkpoint:

```text
dev 0.1.5.7 — Known-host multi-key portability polish
dev 0.1.5.8 — Windows libssh handshake compatibility polish
dev 0.1.5.9 — Stabilization docs and release polish
```

That checkpoint covers `windeployqt`, copying vcpkg DLLs, running without manually extending `PATH`, and copying the finished `dist\windows-release` folder to a clean Windows 10 machine.

---

## 10. Current Windows build checklist

```text
[ ] cl works in x64 Native Tools prompt
[ ] Qt MSVC path exists
[ ] Qt WebEngine exists
[ ] Qt WebChannel exists
[ ] Qt Positioning exists
[ ] vcpkg libssh installed
[ ] vcpkg pkgconf installed
[ ] CMake Debug configure passes
[ ] CMake Debug build passes
[ ] app launches
[ ] About shows expected version
[ ] Settings opens
[ ] config path is AppData/Local/DD-LAB/DD-SSH
[ ] password SSH connection works
[ ] xterm terminal opens
[ ] htop works
[x] Release configure passes
[x] Release build passes
[ ] startup/RAM notes recorded
[x] Windows deploy-folder tested on Windows 10/11
[x] Windows libssh KEX compatibility regression validated
```


## Windows icon resource

From `dev 0.1.5.2`, DD-SSH includes a Windows `.rc` file and multi-size `.ico` generated from the project icon:

```text
resources/windows/dd-ssh.rc
resources/windows/dd-ssh.ico
```

CMake includes the `.rc` file only on Windows, so the built `.exe` should use the DD-SSH icon instead of the default generic executable icon.

## WebEngine startup note

`dev 0.1.5.3` adds a clearer startup message inside new xterm.js terminal tabs. On Windows, the first terminal tab may take several seconds while Qt WebEngine initializes its Chromium-based runtime, JavaScript engine, WebChannel bridge, and graphics pipeline. This is expected for the current architecture. Later terminal tabs usually open much faster because the WebEngine runtime is already warm.

For public-alpha testing, record:

```text
App startup time:
First terminal tab startup time:
Second terminal tab startup time:
Task Manager RAM after app launch:
Task Manager RAM after first terminal:
```

This is not currently treated as a release blocker unless the terminal fails to load, the app remains permanently not responding, or subsequent tabs remain slow after the first WebEngine initialization.

---

## Windows standalone deployment test

`dev 0.1.5.9` documents the successful standalone Windows deployment flow, the known-host multi-key portability fix, and the Windows-only libssh KEX compatibility override for newer OpenSSH servers.

See:

```text
docs/WINDOWS_DEPLOYMENT.md
scripts/windows-deploy-release.bat
```

The standalone deployment test uses `windeployqt`, copies vcpkg runtime DLLs, and creates:

```text
dist\windows-release\
```

The goal is to run `dd-ssh.exe` from that folder without manually adding Qt or vcpkg paths to `PATH`.
