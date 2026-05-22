# DD-SSH macOS Deployment Guide

**Checkpoint:** dev 0.1.6.2 — Andromeda  
**Phase:** macOS Intel app/DMG foundation

This guide documents the first deployable macOS package path for DD-SSH.

The goal of this checkpoint is practical tester distribution:

```text
DD-SSH.app
DD-SSH-0.1.6.2-macOS-x86_64.dmg
```

The generated DMG contains:

```text
DD-SSH.app
Applications -> /Applications
```

The user opens the DMG and drags `DD-SSH.app` to the `Applications` shortcut.

---

## 1. Build first

```bash
cd ~/DD-SSH
./scripts/macos-build-release.sh
```

Expected build output:

```text
build-macos-release/dd-ssh.app
```

---

## 2. Create deployable app and DMG

```bash
./scripts/macos-deploy-release.sh
```

Default settings:

```text
QT_DIR=$HOME/Qt/6.11.1/macos
BUILD_DIR=build-macos-release
DIST_DIR=dist/macos
MACOS_ARCH=x86_64
DD_SSH_MACOS_VERSION=0.1.6.2
CODESIGN_ADHOC=1
```

Expected output:

```text
dist/macos/DD-SSH.app
dist/macos/DD-SSH-0.1.6.2-macOS-x86_64.dmg
```

---

## 3. What the deployment script does

`scripts/macos-deploy-release.sh` performs these steps:

```text
1. Builds the app if build-macos-release/dd-ssh.app is missing.
2. Copies dd-ssh.app to dist/macos/DD-SSH.app.
3. Runs macdeployqt to bundle Qt frameworks/plugins/WebEngine resources.
4. Scans the app binary with otool.
5. Copies Homebrew dylibs from /usr/local or /opt/homebrew into Contents/Frameworks.
6. Rewrites dylib references with install_name_tool.
7. Applies an ad-hoc signature for local testing when possible.
8. Creates a DMG staging folder with DD-SSH.app and an Applications symlink.
9. Creates a compressed DMG with hdiutil.
```

Homebrew dylib bundling is needed because `macdeployqt` handles Qt runtime files but does not automatically bundle every non-Qt dependency such as Homebrew `libssh` / OpenSSL.

---

## 4. Local test

Open the deployed app:

```bash
open dist/macos/DD-SSH.app
```

Open the DMG:

```bash
open dist/macos/DD-SSH-0.1.6.2-macOS-x86_64.dmg
```

Then drag `DD-SSH.app` to `Applications` and launch it from there.

---

## 5. Inspect dependencies

Check the app binary:

```bash
otool -L dist/macos/DD-SSH.app/Contents/MacOS/dd-ssh
```

A good deployable app should not depend on personal build paths such as:

```text
/Users/user/Qt/...
/usr/local/Cellar/libssh/...
/usr/local/opt/openssl@3/...
```

Qt framework references may use bundle-relative paths, and Homebrew libraries should be copied into:

```text
DD-SSH.app/Contents/Frameworks/
```

If a tester Mac reports `Library not loaded`, run `otool -L` and look for the missing path.

---

## 6. Gatekeeper and signing notes

This first DMG is intentionally **unsigned and not notarized**.

Expected tester behavior:

```text
Right-click DD-SSH.app -> Open
```

or, for internal testing only:

```bash
xattr -dr com.apple.quarantine /Applications/DD-SSH.app
open /Applications/DD-SSH.app
```

Future release work should add:

```text
Developer ID signing
hardened runtime where needed
notarization with notarytool
stapling
Homebrew Cask packaging
```

---

## 7. macOS version notes

The first validated build machine is:

```text
macOS 15.7.5
Intel x86_64
Qt 6.11.1
```

Recommended initial support statement:

```text
macOS support is experimental.
Initial package: Intel x86_64 DMG.
Target: modern macOS / Qt 6.11-compatible systems.
Apple Silicon may run the Intel app through Rosetta 2.
Native arm64 and universal builds are planned.
macOS 12.x is not the primary target for this Qt 6.11.1 build.
```

Older macOS compatibility should be handled as a separate legacy-build experiment, not by weakening the main packaging path.

---

## 8. Tester checklist

On a clean tester Mac:

```text
[ ] DMG opens
[ ] DD-SSH.app can be dragged to Applications
[ ] App opens via right-click -> Open if Gatekeeper blocks first launch
[ ] About shows dev 0.1.6.2
[ ] Settings opens
[ ] Config path is created
[ ] Saved session can be created/imported
[ ] Password SSH login works
[ ] Private-key SSH login works
[ ] xterm.js terminal opens
[ ] whoami works
[ ] htop/top works
[ ] Exit safety still prompts when connected tabs are open
```
