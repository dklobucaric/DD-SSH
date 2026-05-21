# DD-SSH Release Checklist

**Current target line:** Andromeda / MF 0.2 candidate  
**Current checkpoint:** dev 0.1.6.1

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
set(DD_SSH_VERSION_STRING "dev 0.1.6.1")
```

Also verify in the app:

```text
Help → About DD-SSH
```

Expected:

```text
Version: dev 0.1.6.1
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

For the first Debian package experiment:

```bash
./scripts/linux-package-deb.sh
dpkg-deb -I dist/deb/dd-ssh_0.1.6.1_amd64.deb
dpkg-deb -c dist/deb/dd-ssh_0.1.6.1_amd64.deb | head -50
```

Optional install test on a disposable or safe Linux machine:

```bash
sudo apt install ./dist/deb/dd-ssh_0.1.6.1_amd64.deb
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

## 5. Windows deployed-folder smoke test

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

## 6. Regression checks

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

## 7. Documentation checks

```text
[ ] README current status is accurate
[ ] TEST_MATRIX reflects validated platforms conservatively
[ ] CHANGELOG has the new checkpoint entry
[ ] WINDOWS_DEPLOYMENT matches the checked-in BAT script
[ ] KNOWN_LIMITATIONS does not hide unfinished items
[ ] SECURITY_NOTES still warns about plaintext secrets
```

## 8. Optional internal tag

```bash
git tag dev-0.1.5.9
git push origin dev-0.1.5.9
```

Use public release tags only when the full public-alpha checklist passes.
