# DD-SSH dev 0.1.6.4 — Build and test notes

Checkpoint: `dev 0.1.6.4`  
Scope: repo hygiene and release artifact workflow

This checkpoint intentionally keeps runtime/SSH behavior unchanged from `dev 0.1.6.3`. It adds `.gitignore` protection, release-artifact documentation, and SHA256 checksum helpers for Linux, macOS, and Windows.

## Linux build smoke test

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

Expected:

```text
Help/About shows dev 0.1.6.4
Saved sessions still load
xterm.js terminal still opens
```

## Linux `.deb` package test

```bash
cd ~/DD-SSH
rm -rf build-linux-release dist/deb
./scripts/linux-build-release.sh
DD_SSH_DEB_VERSION=0.1.6.4 ./scripts/linux-package-deb.sh
ls -lah dist/deb/
```

Expected:

```text
dist/deb/dd-ssh_0.1.6.4_amd64.deb
```

Install locally:

```bash
sudo apt remove dd-ssh
sudo apt install ./dist/deb/dd-ssh_0.1.6.4_amd64.deb
dd-ssh
```

## Checksum tests

Linux:

```bash
./scripts/generate-checksums-linux.sh
cat dist/SHA256SUMS
```

macOS:

```bash
./scripts/generate-checksums-macos.sh
cat dist/SHA256SUMS
```

Windows:

```cmd
scripts\generate-checksums-windows.bat
```

Expected:

```text
dist/SHA256SUMS contains release artifacts such as .deb, .dmg, .zip, .AppImage, .msi, .pkg, .tar.gz, or .tgz
```

## `.gitignore` test

Create dummy generated files:

```bash
mkdir -p dist/test build/test
printf test > dist/test/kiflin.zip
printf test > dist/test/kiflin.dmg
printf test > .DS_Store
```

Check status:

```bash
git status --ignored
```

Expected:

```text
dist/ is ignored
build/ is ignored
.DS_Store is ignored
```

Clean dummy files:

```bash
rm -rf dist/test build/test .DS_Store
```

## Windows pull/build/deploy reminder

From `x64 Native Tools Command Prompt for VS 2022`:

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

Then zip `dist\windows-release` before creating checksums:

```cmd
powershell -NoProfile -Command "Compress-Archive -Path dist\windows-release\* -DestinationPath dist\DD-SSH-dev-0.1.6.4-windows-portable.zip -Force"
scripts\generate-checksums-windows.bat
```
