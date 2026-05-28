# DD-SSH dev 0.1.7.1 — Build and test

Purpose: focused native paste event hardening after the 0.1.7.0 terminal transport checkpoint.

## Linux build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Linux package smoke test

```bash
rm -rf build-linux-release dist/deb
./scripts/linux-build-release.sh
DD_SSH_DEB_VERSION=0.1.7.1 ./scripts/linux-package-deb.sh
sudo apt install ./dist/deb/dd-ssh_0.1.7.1_amd64.deb
dd-ssh
```

## Windows smoke test

Pull the pushed commit, build Release, run the app from the build folder, then run:

```cmd
scripts\windows-deploy-release.bat
```

Test the standalone folder from a normal Command Prompt without development PATH changes.

## macOS smoke test

```bash
cd ~/DD-SSH
rm -rf build-macos-release dist/macos
./scripts/macos-build-release.sh
./scripts/macos-deploy-release.sh
open dist/macos/DD-SSH-0.1.7.1-macOS-x86_64.dmg
```

## Expected result

About should show `dev 0.1.7.1`. Right-click paste and Ctrl+Shift+V should paste cleanly without literal bracketed paste markers.
