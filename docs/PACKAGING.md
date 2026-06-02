# Packaging DD-SSH

**Checkpoint:** dev 0.1.8.7 — Andromeda
**Phase:** Release/tester packaging polish

DD-SSH has moved from source-build validation into a tester packaging phase. Generated artifacts stay out of Git, and current tester targets are Linux `.deb`, Windows portable ZIP, and unsigned Intel macOS `.dmg` builds.

The first Linux package target is a practical `.deb` for Debian/Ubuntu/Mint/LMDE-style systems. Windows already has a validated deploy-folder flow; a Windows installer is planned later.

## Current package targets

### Linux

Current target:

- `.deb` package experiment

Scripts:

```text
scripts/linux-build-release.sh
scripts/linux-deploy-release.sh
scripts/linux-package-deb.sh
```

Documentation:

```text
docs/LINUX_PACKAGING.md
```

The first `.deb` installs DD-SSH under the normal `/usr` layout:

```text
/usr/bin/dd-ssh
/usr/share/applications/dd-ssh.desktop
/usr/share/icons/hicolor/.../apps/dd-ssh.png
/usr/share/doc/dd-ssh/
```

It uses system Qt/libssh runtime packages instead of bundling Qt libraries.

### Windows

Current target:

- portable deploy folder

Script:

```text
scripts/windows-deploy-release.bat
```

Planned later:

- Inno Setup or NSIS installer
- Start Menu shortcut
- optional Desktop shortcut
- uninstaller
- signing later

### macOS

Current target:

- Intel `.app` bundle
- unsigned Intel `.dmg`
- dependency audit report generated from `otool`

Planned later:

- signing/notarization when appropriate
- native Apple Silicon / universal build
- Homebrew Cask draft

## Build first Linux `.deb`

From the project root on Linux:

```bash
./scripts/linux-build-release.sh
./scripts/linux-package-deb.sh
```

Expected output:

```text
dist/deb/dd-ssh_0.1.8.7_amd64.deb
```

Install locally:

```bash
sudo apt install ./dist/deb/dd-ssh_0.1.8.7_amd64.deb
```

Run:

```bash
dd-ssh
```


## Release artifacts and checksums

Generated packages and deployment folders are ignored by Git. Upload final release packages to GitHub Releases together with `SHA256SUMS`.

Checksum helpers:

```text
scripts/generate-checksums-linux.sh
scripts/generate-checksums-macos.sh
scripts/generate-checksums-windows.bat
scripts/generate-checksums-windows.ps1
```

See [Release Artifacts](RELEASE_ARTIFACTS.md) for the full policy.

## Important packaging notes

DD-SSH uses Qt WebEngine. Packaging must preserve Qt WebEngine runtime compatibility.

xterm.js assets are bundled through Qt resources and should not require network access.

## Config location

Do not package a real `dd-ssh.json` with secrets.

The app creates/uses a per-user config path through Qt standard paths.

## Icons

The Linux `.deb` installs PNG icons into the hicolor icon theme and references `dd-ssh` from the `.desktop` file. Windows deployment preserves the embedded `.exe` icon generated from `resources/windows/dd-ssh.rc`. macOS packaging should include `resources/macos/dd-ssh.icns` inside the app bundle.

## Future targets

Linux future work:

- AppImage
- Snap
- Flatpak maybe later

Windows future work:

- installer
- code signing later

macOS future work:

- clean tester-Mac pass
- Apple Silicon / universal build
- Homebrew Cask draft
- notarization later


## macOS app/DMG packaging

`dev 0.1.6.5` polishes the Intel macOS app/DMG path. See:

- [macOS Build Guide](MACOS_BUILD.md)
- [macOS Deployment Guide](MACOS_DEPLOYMENT.md)

Expected outputs:

```text
build-macos-release/dd-ssh.app
dist/macos/DD-SSH.app
dist/macos/DD-SSH-0.1.6.5-macOS-x86_64.dmg
dist/macos/DD-SSH-0.1.6.5-macOS-x86_64-otool-report.txt
```

This first DMG is intentionally unsigned and not notarized. It is suitable for internal testing and early testers who understand Gatekeeper prompts. Developer ID signing, notarization, Homebrew Cask packaging, and native arm64/universal builds are later packaging tasks.


## Windows portable ZIP helper

`dev 0.1.8.7` adds a small wrapper for packaging the deployed Windows portable folder into a versioned ZIP:

```cmd
scripts\windows-deploy-release.bat
scripts\windows-package-portable.bat
scripts\generate-checksums-windows.bat
```

Expected ZIP style:

```text
dist\DD-SSH-0.1.8.7-Windows-x86_64-portable.zip
```
