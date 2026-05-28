# Testcase — dev 0.1.6.5

**Checkpoint:** dev 0.1.6.5 — Andromeda
**Goal:** validate macOS DMG/dependency polish without changing runtime behavior.

## Test 1 — Linux regression smoke test

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

Pass criteria:

```text
[ ] App launches
[ ] About shows dev 0.1.6.5
[ ] Existing saved sessions load
[ ] Trusted xterm.js session opens
[ ] Wrong-password test still reports auth failure normally
```

## Test 2 — macOS build bundle

```bash
cd ~/DD-SSH
rm -rf build-macos-release
./scripts/macos-build-release.sh
open build-macos-release/dd-ssh.app
```

Pass criteria:

```text
[ ] dd-ssh.app is created
[ ] App launches from build-macos-release
[ ] About shows dev 0.1.6.5
[ ] Settings opens
```

## Test 3 — macOS deploy script

```bash
rm -rf dist/macos
./scripts/macos-deploy-release.sh
```

Pass criteria:

```text
[ ] dist/macos/DD-SSH.app exists
[ ] dist/macos/DD-SSH-0.1.6.5-macOS-x86_64.dmg exists
[ ] dist/macos/DD-SSH-0.1.6.5-macOS-x86_64-otool-report.txt exists
[ ] Script prints dependency-audit result
```

## Test 4 — DMG layout

```bash
open dist/macos/DD-SSH-0.1.6.5-macOS-x86_64.dmg
```

Pass criteria:

```text
[ ] DMG mounts
[ ] DD-SSH.app is visible
[ ] Applications shortcut is visible
[ ] README_FIRST.txt is visible
[ ] Drag DD-SSH.app to Applications works
```

## Test 5 — dependency audit

```bash
grep -E '/Users/|/usr/local/|/opt/homebrew/' dist/macos/DD-SSH-0.1.6.5-macOS-x86_64-otool-report.txt || echo "Dependency audit looks clean"
```

Pass criteria:

```text
[ ] No unexpected personal build paths remain
[ ] Homebrew libssh/OpenSSL/zlib references are bundled or rewritten
```

If warnings remain, inspect the audit report before distributing the DMG.

## Test 6 — strict dependency audit

```bash
STRICT_DEP_AUDIT=1 ./scripts/macos-deploy-release.sh
```

Pass criteria:

```text
[ ] Script succeeds if dependency audit is clean
[ ] Script fails if unresolved local/Homebrew paths remain
```

## Test 7 — tester Mac launch

On another Mac if available:

```text
[ ] DMG opens
[ ] App copies to Applications
[ ] Right-click / Control-click -> Open works if Gatekeeper blocks first launch
[ ] App launches without installed Qt
[ ] App launches without Homebrew libssh if possible
[ ] SSH password login works
[ ] SSH private-key login works
[ ] xterm.js terminal opens
[ ] whoami works
```
