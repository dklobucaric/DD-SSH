# Packaging DD-SSH

Packaging is not ready yet.

DD-SSH currently focuses on source builds and local testing.

## Future targets

Linux:

- AppImage
- `.deb`
- Snap
- Flatpak maybe later

Windows:

- Portable ZIP
- Installer
- Code signing later

macOS:

- `.app` bundle
- `.dmg`
- Notarization later

## Important packaging notes

DD-SSH uses Qt WebEngine. Packaging must include Qt WebEngine runtime dependencies.

xterm.js assets are bundled through Qt resources and should not require network access.

## Config location

Do not package a real `dd-ssh.json` with secrets.

The app creates/uses a per-user config path through Qt standard paths.

## Before packaging

Required first:

- Public alpha docs
- Known limitations
- Security notes
- Cross-platform build validation
- Clean release tag
- Decision about plaintext secrets warning in release notes
