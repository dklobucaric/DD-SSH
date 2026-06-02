# DD-SSH dev 0.1.8.6.1 — Build and test

Goal: verify that macOS and Linux release artifact scripts no longer produce stale `0.1.7.1` filenames when the app version is newer.

## Linux build smoke

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

Expected: About shows `dev 0.1.8.6.1`.

## macOS release artifact smoke

```bash
cd ~/DD-SSH
rm -rf build-macos-release
rm -rf dist/macos
./scripts/macos-build-release.sh
./scripts/macos-deploy-release.sh
```

Expected default outputs:

```text
dist/macos/DD-SSH-0.1.8.6.1-macOS-x86_64.dmg
dist/macos/DD-SSH-0.1.8.6.1-macOS-x86_64-otool-report.txt
```

The deploy script should print:

```text
[DD-SSH] Package version: 0.1.8.6.1
```

## Optional override test

```bash
DD_SSH_MACOS_VERSION=test-version ./scripts/macos-deploy-release.sh
```

Expected: artifact filenames use `test-version`.
