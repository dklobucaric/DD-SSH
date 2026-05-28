# Build and Test — dev 0.1.6.7

**Checkpoint:** dev 0.1.6.7 — Andromeda
**Scope:** Basic Session Traffic Monitor

This checkpoint adds a compact live traffic indicator in the status bar for the active terminal tab. It is intentionally terminal-channel only for now and does not measure global OS network traffic.

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
About shows dev 0.1.6.7
Status bar initially shows Traffic: No active session
```

## Linux release / .deb smoke

```bash
cd ~/DD-SSH
rm -rf build-linux-release dist/deb
./scripts/linux-build-release.sh
DD_SSH_DEB_VERSION=0.1.6.7 ./scripts/linux-package-deb.sh
sudo apt install ./dist/deb/dd-ssh_0.1.6.7_amd64.deb
```

Run:

```bash
dd-ssh
```

## Windows build reminder

Use `x64 Native Tools Command Prompt for VS 2022`:

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

Standalone smoke from ordinary CMD:

```cmd
cd /d C:\dev\DD-SSH\dist\windows-release
dd-ssh.exe
```

## macOS build reminder

```bash
cd ~/DD-SSH
git pull --ff-only origin main
rm -rf build-macos-release dist/macos
./scripts/macos-build-release.sh
./scripts/macos-deploy-release.sh
```

Expected DMG:

```text
dist/macos/DD-SSH-0.1.6.7-macOS-x86_64.dmg
```

## Traffic monitor smoke test

1. Start DD-SSH.
2. Confirm the status bar says `Traffic: No active session`.
3. Open a saved xterm.js terminal.
4. Confirm the status bar shows the active session name.
5. Run commands with output, for example `ls -laR /etc | head -300` or `top` / `htop`.
6. Confirm received rate and total increase.
7. Type commands or paste a small command.
8. Confirm sent total increases.
9. Open a second terminal tab.
10. Switch tabs and confirm the status bar follows the active tab.
11. Disconnect and confirm the total remains visible with a disconnected state.

## Diagnostic logging smoke test

With Settings → Enable diagnostic logging enabled, open and close a terminal session.

Expected log examples:

```text
Traffic monitor started: session="doma"
Session traffic summary: session="doma", duration=00:04:12, received=3.8 MB, sent=220 KB
Traffic monitor stopped: session="doma", received=3.8 MB, sent=220 KB
```

The log must not contain terminal input, terminal output, passwords, private-key contents, clipboard contents, or full JSON config data.
