# Windows Standalone Deployment Test

**Checkpoint:** dev 0.1.5.6 — Andromeda  
**Purpose:** create and validate a standalone Windows deployment folder that can run outside the build tree and without manually extending `PATH`.

This is still **not a final installer**. It is a portable release-folder test for the Andromeda public-alpha line. The goal is to prove that a Windows-built DD-SSH can be copied into one folder with the required Qt, Qt WebEngine, libssh, OpenSSL, compiler runtime, and helper DLLs.

---

## Deployment goal

Starting point:

```text
C:\dev\DD-SSH\build-win-release\dd-ssh.exe
```

Target folder:

```text
C:\dev\DD-SSH\dist\windows-release\
```

Expected result:

```text
dist\windows-release\dd-ssh.exe
```

can be launched directly from Explorer or from a new normal Command Prompt without setting:

```cmd
PATH=C:\Qt\...;C:\dev\vcpkg\...
```

The final proof is copying the whole `dist\windows-release` folder to a clean Windows 10 machine with no Qt/vcpkg/MSVC development environment and launching `dd-ssh.exe` there.

---

## Required paths used in the validated Windows environment

```text
Qt:     C:\Qt\6.11.1\msvc2022_64
vcpkg:  C:\dev\vcpkg
build:  C:\dev\DD-SSH\build-win-release
output: C:\dev\DD-SSH\dist\windows-release
```

The helper script allows overriding these values:

```cmd
set QT_DIR=C:\Qt\6.11.1\msvc2022_64
set VCPKG_ROOT=C:\dev\vcpkg
set BUILD_DIR=build-win-release
set DIST_DIR=dist\windows-release
```

---

## 1. Build Release first

From **x64 Native Tools Command Prompt for VS 2022**:

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

Verify that this file exists:

```cmd
build-win-release\dd-ssh.exe
```

The dev machine may still need Qt/vcpkg paths to run directly from the build tree. The deployment folder test below should remove that requirement.

---

## 2. Run the deployment helper

From the repo root:

```cmd
scripts\windows-deploy-release.bat
```

The script does this:

```text
1. removes and recreates dist\windows-release
2. copies dd-ssh.exe
3. runs windeployqt with --release --compiler-runtime --webengine
4. copies vcpkg runtime DLLs from C:\dev\vcpkg\installed\x64-windows\bin
5. runs sanity checks for key runtime files/folders
6. prints the exact launch command for a clean Command Prompt
```

The script is intentionally broad for this alpha checkpoint: it copies all vcpkg runtime DLLs from the vcpkg runtime bin folder. Later packaging can reduce this to the exact required DLL set.

---

## 3. Manual deployment command

If the helper script is not used:

```cmd
cd C:\dev\DD-SSH
rmdir /s /q dist\windows-release
mkdir dist\windows-release
copy build-win-release\dd-ssh.exe dist\windows-release\

C:\Qt\6.11.1\msvc2022_64\bin\windeployqt.exe --release --compiler-runtime --webengine dist\windows-release\dd-ssh.exe
copy C:\dev\vcpkg\installed\x64-windows\bin\*.dll dist\windows-release\
```

---

## 4. Local no-PATH test

Open a **new normal Command Prompt**, not the VS developer prompt.

Do not manually add Qt or vcpkg to `PATH`.

Run:

```cmd
cd /d "C:\dev\DD-SSH\dist\windows-release"
dd-ssh.exe
```

Expected:

```text
- DD-SSH opens
- Help → About shows dev 0.1.5.6
- app icon appears
- Settings opens
- config path points to AppData\Local\DD-LAB\DD-SSH
- double-click saved session opens xterm.js terminal
```

If a DLL is missing, Windows will usually report it during startup. Add that DLL source/path to this document and the deployment script.

---

## 5. Clean Windows 10 machine test

Copy the whole folder:

```text
dist\windows-release
```

to the clean Windows 10 test machine. Do not install Qt, vcpkg, Visual Studio Build Tools, or Ninja on that machine for this test.

Run:

```cmd
dd-ssh.exe
```

Minimum test pass:

```text
[ ] app launches
[ ] Help → About opens and shows dev 0.1.5.6
[ ] Settings opens
[ ] config path is under AppData\Local\DD-LAB\DD-SSH
[ ] new saved password session can be created after successful auth
[ ] saved session appears in the sidebar
[ ] double-click saved session opens xterm.js terminal
[ ] whoami works
[ ] htop works
[ ] disconnect works
[ ] reconnect works
[ ] app icon is visible on window/taskbar/Explorer where applicable
[ ] closing with an active SSH session asks before exit
```

Optional extra pass:

```text
[ ] private-key auth works
[ ] config export works
[ ] config import works
[ ] first terminal startup time recorded
[ ] second terminal startup time recorded
[ ] RAM usage after first terminal recorded
```

---

## 6. Runtime checks from deployed folder

Use either an imported config or create a new test session.

Check:

```text
Help → About DD-SSH
Tools → Settings
File → Open Config Folder
File → Export Config
Session → New Session
Double-click saved session
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
- Disconnect
- Reconnect
- first terminal loading message
- second terminal opens faster than the first
```

---

## 7. Expected deployment contents

The deployment folder will be much larger than `dd-ssh.exe` because Qt WebEngine includes a Chromium-based runtime.

Expected contents include some of:

```text
dd-ssh.exe
Qt6*.dll
libEGL.dll / libGLESv2.dll
D3D compiler / graphics helper DLLs
QtWebEngineProcess.exe
resources\
translations\
imageformats\
platforms\
styles\
tls\
ssh.dll or libssh.dll
libcrypto*.dll
libssl*.dll
zlib*.dll
```

Exact filenames depend on Qt and vcpkg versions.

---

## 8. Known Windows deployment notes

- Qt WebEngine deployment is larger than a plain Qt Widgets app.
- The first terminal tab may still take several seconds while WebEngine initializes.
- Runtime RAM use with one WebEngine terminal may be hundreds of MB.
- Qt may create cache folders under the DD-SSH AppData location.
- This deployment test does not create an installer yet.
- This deployment test is not code-signed.
- If the clean machine reports a missing DLL, add that DLL to the deployment script notes before calling the checkpoint passed.

---

## 9. Pass criteria for dev 0.1.5.6

Mark this checkpoint as passed when:

```text
[x] Release build succeeds
[x] deployment folder is created
[x] app starts from deployment folder without Qt/vcpkg PATH
[x] About shows dev 0.1.5.6
[x] app icon appears in Explorer/taskbar/window
[x] Settings opens and saves
[x] xterm terminal opens
[x] SSH password login works
[x] whoami works
[x] htop works
[x] Disconnect/Reconnect still work
[x] clean Windows 10 machine launches dd-ssh.exe from copied folder
```
