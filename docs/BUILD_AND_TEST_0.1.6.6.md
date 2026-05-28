# Build and Test — dev 0.1.6.6

**Checkpoint:** dev 0.1.6.6 — Andromeda
**Goal:** optional diagnostic logging foundation

This checkpoint should be tested as a normal DD-SSH source release. Runtime SSH logic is intentionally unchanged from dev 0.1.6.3; the focus is Settings/Help/logging behavior.

## Linux build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

Expected:

```text
About shows dev 0.1.6.6
Settings contains Enable diagnostic logging
Help contains Open Log Folder
logging is OFF by default
```

## Linux Release build / .deb

```bash
cd ~/DD-SSH
rm -rf build-linux-release dist/deb
./scripts/linux-build-release.sh
DD_SSH_DEB_VERSION=0.1.6.6 ./scripts/linux-package-deb.sh
sudo apt install ./dist/deb/dd-ssh_0.1.6.6_amd64.deb
dd-ssh
```

Expected:

```text
About shows dev 0.1.6.6
Help → Open Log Folder opens ~/.local/state/DD-SSH/logs
```

## Windows pull/build/deploy

Use x64 Native Tools Command Prompt for VS 2022:

```cmd
cd /d C:\dev\DD-SSH
git pull --ff-only origin main

rmdir /s /q build-win-release
rmdir /s /q dist\windows-release

cmake -S . -B build-win-release -G Ninja ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_PREFIX_PATH=C:\Qt\6.11.1\msvc2022_64 ^
  -DCMAKE_TOOLCHAIN_FILE=C:\dev\vcpkg\scripts\buildsystems\vcpkg.cmake ^
  -DPKG_CONFIG_EXECUTABLE=C:\dev\vcpkg\installed\x64-windows\tools\pkgconf\pkgconf.exe

cmake --build build-win-release
scripts\windows-deploy-release.bat
```

Standalone smoke test from a normal Command Prompt:

```cmd
cd /d C:\dev\DD-SSH\dist\windows-release
dd-ssh.exe
```

Expected log folder when enabled:

```text
%LOCALAPPDATA%\DD-SSH\logs
```

## macOS build / DMG

```bash
cd ~/DD-SSH
git pull --ff-only origin main
rm -rf build-macos-release dist/macos
./scripts/macos-build-release.sh
./scripts/macos-deploy-release.sh
```

Expected:

```text
dist/macos/DD-SSH.app
dist/macos/DD-SSH-0.1.6.6-macOS-x86_64.dmg
```

Expected log folder when enabled:

```text
~/Library/Logs/DD-SSH
```

## Important test rule

Diagnostic logging is a debug tool and is OFF by default. A fresh launch must not create a log file unless Settings → Enable diagnostic logging has been enabled and saved.
