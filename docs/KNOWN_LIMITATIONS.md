# DD-SSH Known Limitations

**Checkpoint:** dev 0.1.5.8 — Andromeda

This document lists limitations that should be visible to testers. Nothing here is hidden or sugar-coated.

---

## Security limitations

- Saved passwords may be stored in plaintext in `dd-ssh.json`.
- Saved private keys may be stored in plaintext in `dd-ssh.json`.
- There is no master password yet.
- There is no encrypted secrets store yet.
- There is no SSH agent integration yet.
- Use only on trusted machines.

---

## Platform limitations

- Linux is the primary tested platform.
- Native Windows Debug and Release builds have been validated for app launch and SSH/xterm workflows. Standalone deployment-folder validation passed on real Windows 10/11 machines; current 0.1.5.8 focus is Windows libssh handshake compatibility while retaining the 0.1.5.7 known-host portability fix.
- macOS builds are planned but not fully validated.
- Installers are not ready; the current Windows target is a copied `dist\windows-release` folder.
- Code signing/notarization is not ready.

---

## Feature limitations

Not implemented yet:

- SFTP
- Split panes
- Multi-Exec
- Keep-alive settings
- Portable mode next to binary
- Custom config path picker
- Encrypted config migration
- Advanced theme editor
- Terminal theme editor

---

## Terminal limitations

The xterm.js terminal foundation is working, including PTY resize and several fullscreen terminal apps. Still, testers should report issues with:

- unusual key combinations
- function keys
- alternate keyboard layouts
- tmux/screen edge cases
- terminal resize edge cases
- copy/paste edge cases

---

## Config limitations

Config import/export and recovery are implemented, but testers should handle real configs carefully because the file may contain plaintext secrets.

Always keep backups before testing import/export/recovery behavior.


---

## Windows-specific alpha notes

The native Windows build is confirmed to launch and run SSH/xterm workflows. The current Windows work is validating a standalone deploy folder, not a full installer.

Known Windows alpha notes:

- Debug build startup is slower than Linux.
- First xterm.js terminal startup can take several seconds because Qt WebEngine initializes lazily.
- Subsequent terminal tabs are much faster.
- RAM usage can be hundreds of MB with an active xterm/WebEngine terminal because Qt WebEngine embeds Chromium components.
- Qt may create a cache folder under the DD-SSH AppData directory.
- `windeployqt` deployment helper exists; clean Windows 10 machine validation is still required before calling the deploy-folder checkpoint passed.

## Known-host portability note

`dev 0.1.5.7` fixes the single-key known-host portability limitation found during Windows standalone testing. DD-SSH now accepts the legacy one-key format but saves known-host entries with a `keys` object so ED25519 and ECDSA host keys for the same `host:port` can coexist.

A true same-algorithm fingerprint mismatch is still treated as a strong host-key-changed warning.
