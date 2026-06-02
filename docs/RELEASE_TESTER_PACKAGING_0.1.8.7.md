# Release/tester packaging checklist — dev 0.1.8.7

Checkpoint: `dev 0.1.8.7` — Release/tester packaging polish.

This checklist prepares Linux, Windows, and macOS tester artifacts from the same pushed source baseline.

## General preflight

```bash
git status
git checkout main
git pull --ff-only origin main
git status
```

Confirm the app reports:

```text
dev 0.1.8.7
Codename: Andromeda
Milestone: Release/tester packaging polish
```

Generated artifacts must not be committed back to Git. Keep `build-*`, `dist/`, `.deb`, `.dmg`, `.zip`, `.AppImage`, `.pkg`, `.msi`, and checksum outputs as release/tester artifacts only.

## Linux Release + .deb

```bash
cd ~/DD-SSH
rm -rf build-linux-release dist/linux-release dist/deb
chmod +x scripts/linux-build-release.sh
chmod +x scripts/linux-deploy-release.sh
chmod +x scripts/linux-package-deb.sh
chmod +x scripts/generate-checksums-linux.sh
./scripts/linux-build-release.sh
./scripts/linux-deploy-release.sh
./scripts/linux-package-deb.sh
./scripts/generate-checksums-linux.sh
```

Expected package style:

```text
dist/deb/dd-ssh_0.1.8.7_amd64.deb
dist/SHA256SUMS
```

Smoke test:

```bash
./build-linux-release/dd-ssh
# or install the .deb on a test machine
sudo apt install ./dist/deb/dd-ssh_0.1.8.7_amd64.deb
dd-ssh
```

## macOS Intel .app / .dmg

```bash
cd ~/DD-SSH
git status
git checkout main
git pull --ff-only origin main
rm -rf build-macos-release dist/macos
chmod +x scripts/macos-build-release.sh
chmod +x scripts/macos-deploy-release.sh
chmod +x scripts/generate-checksums-macos.sh
./scripts/macos-build-release.sh
open "$HOME/DD-SSH/build-macos-release/dd-ssh.app"
./scripts/macos-deploy-release.sh
./scripts/generate-checksums-macos.sh
```

Expected artifact style:

```text
dist/macos/DD-SSH-0.1.8.7-macOS-x86_64.dmg
dist/macos/DD-SSH-0.1.8.7-macOS-x86_64-otool-report.txt
dist/SHA256SUMS
```

Gatekeeper note: this tester build is unsigned/not notarized. Use right-click / Control-click -> Open if macOS blocks first launch.

## Windows portable folder + ZIP

From a Windows developer shell where Qt, MSVC, Ninja, Git, and vcpkg/libssh are available:

```cmd
cd /d C:\path\to\DD-SSH
git status
git checkout main
git pull --ff-only origin main
rmdir /s /q build-win-release 2>nul
rmdir /s /q dist\windows-release 2>nul
cmake -S . -B build-win-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:\Qt\6.11.1\msvc2022_64 -DCMAKE_TOOLCHAIN_FILE=C:\dev\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build build-win-release
scripts\windows-deploy-release.bat
scripts\windows-package-portable.bat
scripts\generate-checksums-windows.bat
```

Expected artifact style:

```text
dist\DD-SSH-0.1.8.7-Windows-x86_64-portable.zip
dist\SHA256SUMS
```

Smoke test the extracted portable folder on a clean Windows 10/11 machine if possible.

## Cross-platform smoke checklist

- About shows `dev 0.1.8.7`.
- Welcome screen lists terminal + File Manager alpha features.
- Open terminal from a saved session.
- Paste multiline YAML into `nano`; line breaks and indentation are preserved.
- Open File Manager.
- Queue upload/download of harmless test files.
- Folder upload/download with a small test folder.
- Retry selected.
- Overwrite metadata dialog.
- Delete local/delete remote for harmless test files only.
- Enable diagnostic logging and verify transfer/delete events are logged without secrets or file contents.
- Close app with open terminal/File Manager/queue state and verify exit safety prompts.

## Tester handoff bundle

For each platform, provide:

- the package/archive,
- `SHA256SUMS`,
- `docs/TESTER_CHECKLIST_0.1.8.7.md`,
- `docs/KNOWN_LIMITATIONS.md`,
- a reminder to use harmless test folders first.
