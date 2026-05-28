# Build and Test — dev 0.1.6.5

**Checkpoint:** dev 0.1.6.5 — Andromeda
**Phase:** macOS DMG/dependency polish

This checkpoint does **not** change SSH/session/terminal runtime behavior. It improves the macOS tester deployment flow and documentation.

## Scope

Changed:

```text
CMakeLists.txt version string
scripts/macos-deploy-release.sh
macOS build/deployment docs
roadmap/changelog/test documentation
```

Not changed:

```text
SSH core
known_hosts logic
plain-v1 JSON format
terminal/xterm.js runtime
traffic widget
file manager/SFTP
Windows installer
Linux .deb packaging logic
```

## Linux smoke build

The first sanity check can still run on Linux even though this is a macOS packaging checkpoint:

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

Expected:

```text
About shows dev 0.1.6.5
Saved sessions still load
xterm.js terminal still opens
```

## macOS build

On the Intel Mac build machine:

```bash
cd ~/DD-SSH
git pull --ff-only origin main
rm -rf build-macos-release dist/macos
./scripts/macos-build-release.sh
open build-macos-release/dd-ssh.app
```

Expected:

```text
build-macos-release/dd-ssh.app exists
About shows dev 0.1.6.5
App launches from the build bundle
```

## macOS deployment and DMG

```bash
./scripts/macos-deploy-release.sh
```

Expected output:

```text
dist/macos/DD-SSH.app
dist/macos/DD-SSH-0.1.6.5-macOS-x86_64.dmg
dist/macos/DD-SSH-0.1.6.5-macOS-x86_64-otool-report.txt
```

Open the deployed app:

```bash
open dist/macos/DD-SSH.app
```

Open the DMG:

```bash
open dist/macos/DD-SSH-0.1.6.5-macOS-x86_64.dmg
```

DMG should contain:

```text
DD-SSH.app
Applications -> /Applications
README_FIRST.txt
```

## Dependency audit

Inspect the generated audit:

```bash
cat dist/macos/DD-SSH-0.1.6.5-macOS-x86_64-otool-report.txt
```

Quick check for problematic local paths:

```bash
grep -E '/Users/|/usr/local/|/opt/homebrew/' dist/macos/DD-SSH-0.1.6.5-macOS-x86_64-otool-report.txt || echo "Dependency audit looks clean"
```

Strict deployment check:

```bash
STRICT_DEP_AUDIT=1 ./scripts/macos-deploy-release.sh
```

With strict mode, the script exits with an error if local/Homebrew paths remain after bundling.

## Gatekeeper tester note

This build is still unsigned/not notarized. On a tester Mac, first launch may require:

```text
Right-click / Control-click DD-SSH.app -> Open -> Open
```

For internal testing only:

```bash
xattr -dr com.apple.quarantine /Applications/DD-SSH.app
open /Applications/DD-SSH.app
```

## Checksum

After the DMG is created:

```bash
./scripts/generate-checksums-macos.sh
cat dist/SHA256SUMS
```

The DMG should appear in `SHA256SUMS`.
