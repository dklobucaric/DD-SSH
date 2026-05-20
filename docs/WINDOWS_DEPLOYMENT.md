# Windows Deployment Experiment

**Checkpoint:** dev 0.1.5.4 — Andromeda  
**Purpose:** create a first standalone Windows deployment folder that can run outside the build tree and without manually extending `PATH`.

This is an **experiment**, not a final installer. The goal is to prove that a Windows-built DD-SSH can be copied into a deploy folder with the required Qt, Qt WebEngine, libssh, OpenSSL, and runtime DLLs.

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

can be launched directly from Explorer or a clean command prompt, without setting:

```cmd
PATH=C:\Qt\...;C:\dev\vcpkg\...
```

---

## Required paths used in the first Windows validation

```text
Qt:     C:\Qt\6.11.1\msvc2022_64
vcpkg:  C:\dev\vcpkg
build:  C:\dev\DD-SSH\build-win-release
output: C:\dev\DD-SSH\dist\windows-release
```

The helper script allows overriding `QT_DIR`, `VCPKG_ROOT`, `BUILD_DIR`, and `DIST_DIR` if needed.

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

Verify:

```cmd
build-win-release\dd-ssh.exe
```

runs when Qt and vcpkg DLL folders are temporarily added to `PATH`.

---

## 2. Run the deployment helper

From the repo root:

```cmd
scripts\windows-deploy-release.bat
```

The script does this:

```text
1. creates dist\windows-release
2. copies dd-ssh.exe
3. runs windeployqt with --release --webengine
4. copies vcpkg runtime DLLs from C:\dev\vcpkg\installed\x64-windows\bin
5. prints the output folder
```

The script is intentionally broad for the first experiment: it copies vcpkg DLLs from the vcpkg runtime bin folder. Later release packaging can reduce this to only the exact required DLL set.

---

## 3. Manual deployment command

If you do not want to use the helper script:

```cmd
cd C:\dev\DD-SSH
rmdir /s /q dist\windows-release
mkdir dist\windows-release
copy build-win-release\dd-ssh.exe dist\windows-release\

C:\Qt\6.11.1\msvc2022_64\bin\windeployqt.exe --release --compiler-runtime --webengine dist\windows-release\dd-ssh.exe
copy C:\dev\vcpkg\installed\x64-windows\bin\*.dll dist\windows-release\
```

---

## 4. Clean-machine style test

Open a **new normal Command Prompt**, not the VS developer prompt.

Do not set Qt/vcpkg `PATH`.

Run:

```cmd
cd C:\dev\DD-SSH\dist\windows-release
dd-ssh.exe
```

Expected:

```text
- DD-SSH opens
- app icon appears
- Help → About works
- Settings opens
- config path points to AppData\Local\DD-LAB\DD-SSH
- double-click saved session opens xterm terminal
```

If a DLL is missing, Windows will usually report it during startup. Add that DLL source/path to this document and the deployment script.

---

## 5. Runtime checks from deployed folder

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

## 6. Expected deployment contents

The deployment folder will be larger than `dd-ssh.exe` because Qt WebEngine includes a Chromium-based runtime.

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

## 7. Known Windows deployment notes

- Qt WebEngine deployment is larger than a plain Qt Widgets app.
- The first terminal tab may still take several seconds while WebEngine initializes.
- Runtime RAM use with one WebEngine terminal may be hundreds of MB.
- Qt may create cache folders under the DD-SSH AppData location.
- This deployment experiment does not create an installer yet.
- This deployment experiment is not code-signed.

---

## 8. Pass criteria for dev 0.1.5.4

Mark this checkpoint as passed when:

```text
[x] Release build succeeds
[x] deployment folder is created
[x] app starts from deployment folder without Qt/vcpkg PATH
[x] About shows dev 0.1.5.4
[x] app icon appears in Explorer/taskbar/window
[x] Settings opens and saves
[x] xterm terminal opens
[x] SSH login works
[x] htop works
[x] Disconnect/Reconnect still work
```

