# Packaging DD-SSH

**Checkpoint:** dev 0.1.6.1.1 — Andromeda  
**Phase:** README screenshots and Debian packaging tutorial polish

DD-SSH has moved from source-build validation into the first packaging phase.

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

Planned later:

- `.app` bundle
- `.dmg`
- signing/notarization when appropriate

## Build first Linux `.deb`

From the project root on Linux:

```bash
./scripts/linux-build-release.sh
./scripts/linux-package-deb.sh
```

Expected output:

```text
dist/deb/dd-ssh_0.1.6.1.1_amd64.deb
```

Install locally:

```bash
sudo apt install ./dist/deb/dd-ssh_0.1.6.1.1_amd64.deb
```

Run:

```bash
dd-ssh
```

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

- `.app` bundle
- `.dmg`
- notarization later
