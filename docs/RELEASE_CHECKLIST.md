# DD-SSH Release Checklist

**Current target line:** Andromeda / MF 0.2 candidate  
**Current checkpoint:** dev 0.1.6.9

This checklist is shorter than the full public-alpha checklist. Use it before tagging any internal development checkpoint or before preparing `v0.2.0-alpha`.

## 1. Repository state

```bash
git status
git log --oneline -5
```

Expected:

```text
working tree clean
latest commit matches the intended checkpoint
```

## 2. Version identity

Check:

```bash
grep "DD_SSH_VERSION_STRING" CMakeLists.txt
```

Expected for this checkpoint:

```text
set(DD_SSH_VERSION_STRING "dev 0.1.6.9")
```

Also verify in the app:

```text
Help → About DD-SSH
```

Expected:

```text
Version: dev 0.1.6.9
Codename: Andromeda
Milestone: MF 0.2 candidate
```

## 3. Linux smoke test

```bash
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

Minimum pass:

```text
[ ] app launches
[ ] About shows expected version
[ ] Settings opens
[ ] saved session opens xterm terminal
[ ] whoami works
[ ] htop works
[ ] disconnect/reconnect work
```

## 4. Linux Debian package smoke test

For the macOS Intel app/DMG foundation:

```bash
./scripts/linux-package-deb.sh
dpkg-deb -I dist/deb/dd-ssh_0.1.6.5_amd64.deb
dpkg-deb -c dist/deb/dd-ssh_0.1.6.5_amd64.deb | head -50
```

Optional install test on a disposable or safe Linux machine:

```bash
sudo apt install ./dist/deb/dd-ssh_0.1.6.5_amd64.deb
dd-ssh
sudo apt remove dd-ssh
```

Minimum pass:

```text
[ ] .deb file is created
[ ] package metadata looks sane
[ ] package contains /usr/bin/dd-ssh
[ ] package contains /usr/share/applications/dd-ssh.desktop
[ ] package contains hicolor icons
[ ] installed dd-ssh launches
[ ] About shows expected version
[ ] user config is preserved after install/remove
```

## 5. Windows build smoke test

From **x64 Native Tools Command Prompt for VS 2022**:

```cmd
cd C:\dev\DD-SSH
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

Minimum pass:

```text
[ ] CMake finds MSVC compiler
[ ] CMake finds libssh through pkgconf/vcpkg
[ ] Release exe links
[ ] deployment folder is created
[ ] deployed exe starts
[ ] About shows expected version
```

## 6. Windows deployed-folder smoke test

Open a normal Command Prompt, not the VS developer prompt:

```cmd
cd /d C:\dev\DD-SSH\dist\windows-release
dd-ssh.exe
```

Minimum pass:

```text
[ ] app starts without manually setting Qt/vcpkg PATH
[ ] app icon appears
[ ] Settings opens
[ ] imported config loads
[ ] saved session opens xterm terminal
[ ] whoami works
[ ] htop works
[ ] closing with an active SSH session shows exit confirmation
```


## 7. macOS Intel app/DMG smoke test

On the Intel macOS build machine:

```bash
cd ~/DD-SSH
rm -rf build-macos-release dist/macos
./scripts/macos-build-release.sh
./scripts/macos-deploy-release.sh
open dist/macos/DD-SSH.app
open dist/macos/DD-SSH-0.1.6.5-macOS-x86_64.dmg
```

Minimum pass:

```text
[ ] CMake finds Qt 6.11.1 under ~/Qt/6.11.1/macos
[ ] CMake finds Homebrew libssh through pkg-config
[ ] build-macos-release/dd-ssh.app is created
[ ] dist/macos/DD-SSH.app is created
[ ] dist/macos/DD-SSH-0.1.6.5-macOS-x86_64.dmg is created
[ ] DMG opens and shows DD-SSH.app plus Applications shortcut
[ ] About shows expected version
[ ] Settings opens
[ ] saved session opens xterm terminal
[ ] whoami works
```

## 8. Regression checks

Known-host multi-key portability:

```text
[ ] ED25519-only config can add ECDSA as Trust additional key
[ ] ECDSA-only config can add ED25519 as Trust additional key
[ ] final JSON works on Windows 10, Windows 11, and Linux
```

Windows libssh KEX compatibility:

```text
[ ] lab.dd-lab.hr:2231 connects on Windows without server-side KEX workaround
[ ] DD_SSH_DISABLE_WINDOWS_KEX_COMPAT=1 can be used only for comparison/debugging
```

## 9. Documentation checks

```text
[ ] README current status is accurate
[ ] TEST_MATRIX reflects validated platforms conservatively
[ ] CHANGELOG has the new checkpoint entry
[ ] WINDOWS_DEPLOYMENT matches the checked-in BAT script
[ ] KNOWN_LIMITATIONS does not hide unfinished items
[ ] SECURITY_NOTES still warns about plaintext secrets
[ ] MACOS_BUILD and MACOS_DEPLOYMENT match the current scripts
```

## 10. Optional internal tag

```bash
git tag dev-0.1.6.4
git push origin dev-0.1.6.4
```

Use public release tags only when the full public-alpha checklist passes.

## dev 0.1.6.6 diagnostic logging checks

```text
[ ] Settings includes Enable diagnostic logging
[ ] logging is OFF by default
[ ] Help → Open Log Folder works
[ ] status bar shows Logging enabled while enabled
[ ] logs contain INFO/WARN/ERROR lifecycle events
[ ] logs do not contain secrets or terminal content
```


## dev 0.1.6.7 Session Traffic checks

[ ] Status bar shows `Traffic: No active session` on non-terminal tabs.
[ ] Active xterm.js/basic terminal tab shows live received/sent rates and totals.
[ ] Switching terminal tabs changes the active traffic display.
[ ] Disconnect keeps final totals visible with a disconnected state.
[ ] With diagnostic logging ON, traffic started/summary/stopped lines are written.
[ ] Logs do not include terminal input/output or secrets.
