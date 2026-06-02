# Build and test — dev 0.1.8.7

`dev 0.1.8.7` is a release/tester packaging polish checkpoint. It refreshes docs, Welcome/About status, feature inventory, packaging checklist, and tester handoff notes. Runtime SSH/SFTP behavior should remain the accepted `dev 0.1.8.6.4` baseline.

## Linux local build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Linux release/package build

```bash
rm -rf build-linux-release dist/linux-release dist/deb
./scripts/linux-build-release.sh
./scripts/linux-deploy-release.sh
./scripts/linux-package-deb.sh
./scripts/generate-checksums-linux.sh
```

## macOS release build

```bash
rm -rf build-macos-release dist/macos
./scripts/macos-build-release.sh
open "$HOME/DD-SSH/build-macos-release/dd-ssh.app"
./scripts/macos-deploy-release.sh
./scripts/generate-checksums-macos.sh
```

## Windows release package

```cmd
cmake -S . -B build-win-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:\Qt\6.11.1\msvc2022_64 -DCMAKE_TOOLCHAIN_FILE=C:\dev\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build build-win-release
scripts\windows-deploy-release.bat
scripts\windows-package-portable.bat
scripts\generate-checksums-windows.bat
```

## Smoke test

- [ ] About shows `dev 0.1.8.7`.
- [ ] Welcome screen lists current feature set and packaging/tester docs.
- [ ] Terminal opens from saved session.
- [ ] Multiline paste in `nano` preserves formatting.
- [ ] File Manager opens.
- [ ] Queue upload/download/delete works.
- [ ] Folder upload/download small-folder experiment works.
- [ ] Diagnostic logging still works and remains OFF by default.
- [ ] Artifact filenames use `0.1.8.7`.
