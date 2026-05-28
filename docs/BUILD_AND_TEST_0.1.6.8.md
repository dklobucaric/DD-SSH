# Build and Test — dev 0.1.6.8

**Checkpoint:** dev 0.1.6.8 — Andromeda
**Scope:** Config import/export safety preview.

## Linux build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

Expected: Help/About shows `dev 0.1.6.8`.

## Debian package smoke test

```bash
rm -rf build-linux-release dist/deb
./scripts/linux-build-release.sh
DD_SSH_DEB_VERSION=0.1.6.8 ./scripts/linux-package-deb.sh
sudo apt install ./dist/deb/dd-ssh_0.1.6.8_amd64.deb
dd-ssh
```

## Windows smoke test

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

Run from a normal CMD after deploy:

```cmd
cd /d C:\dev\DD-SSH\dist\windows-release
dd-ssh.exe
```

## macOS smoke test

```bash
cd ~/DD-SSH
git pull --ff-only origin main
rm -rf build-macos-release dist/macos
./scripts/macos-build-release.sh
./scripts/macos-deploy-release.sh
open dist/macos/DD-SSH-0.1.6.8-macOS-x86_64.dmg
```

## Focus test

Follow `docs/TESTCASE_0.1.6.8.md`.
