# DD-SSH dev 0.1.6.4 — Testcase

Scope: repo hygiene, release-artifact workflow, and checksum helpers.

This checkpoint should not change SSH runtime behavior. Re-run a small smoke test from `dev 0.1.6.3`, then validate artifact hygiene.

## Test 1 — Linux smoke test

```bash
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

Pass criteria:

- About shows `dev 0.1.6.4`
- Saved sessions load
- Trusted xterm.js session opens
- `whoami` / `clear` work

## Test 2 — generated artifacts are ignored

```bash
mkdir -p dist/test build/test
printf test > dist/test/test.zip
printf test > dist/test/test.deb
printf test > dist/test/test.dmg
printf test > .DS_Store
git status --ignored
```

Pass criteria:

- `dist/` is ignored
- `build/` is ignored
- `.DS_Store` is ignored
- Git does not offer generated artifacts for normal commit

Cleanup:

```bash
rm -rf dist/test build/test .DS_Store
```

## Test 3 — Linux checksum helper

```bash
mkdir -p dist/release-assets
printf zip > dist/release-assets/test.zip
printf deb > dist/release-assets/test.deb
./scripts/generate-checksums-linux.sh
cat dist/SHA256SUMS
```

Pass criteria:

- `dist/SHA256SUMS` is created
- both dummy artifacts are listed
- paths are relative to the repo root

Cleanup:

```bash
rm -rf dist/release-assets dist/SHA256SUMS
```

## Test 4 — macOS checksum helper

On macOS:

```bash
mkdir -p dist/release-assets
printf dmg > dist/release-assets/test.dmg
./scripts/generate-checksums-macos.sh
cat dist/SHA256SUMS
```

Pass criteria:

- script works with macOS `shasum -a 256` fallback if `sha256sum` is unavailable

## Test 5 — Windows checksum helper

On Windows, after creating a portable ZIP:

```cmd
scripts\generate-checksums-windows.bat
```

Pass criteria:

- `dist\SHA256SUMS` is created
- Windows portable `.zip` is listed
- no DLL spam; checksum list should contain release-package artifacts only

## Test 6 — packaging scripts still work

Linux:

```bash
rm -rf build-linux-release dist/deb
./scripts/linux-build-release.sh
DD_SSH_DEB_VERSION=0.1.6.4 ./scripts/linux-package-deb.sh
./scripts/generate-checksums-linux.sh
cat dist/SHA256SUMS
```

Pass criteria:

- `.deb` builds
- `.deb` is included in `SHA256SUMS`
- `git status` does not try to commit `dist/deb/`
