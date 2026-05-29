# DD-SSH Known Limitations

**Checkpoint:** dev 0.1.8.5 — Andromeda

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
- Native Windows Debug and Release builds have been validated for app launch and SSH/xterm workflows. Standalone deployment-folder validation passed on real Windows 10/11 machines. Known-host multi-key portability and Windows libssh KEX compatibility have also been validated.
- macOS Intel builds and a first unsigned `.dmg` workflow have been validated locally, but clean tester-Mac coverage is still limited.
- Windows installer packaging is not ready; the current Windows target is a copied `dist\windows-release` folder.
- macOS code signing/notarization is not ready; testers may need right-click → Open for unsigned apps.

---

## Feature limitations

`dev 0.1.8.5` is a remote queue delete experiment checkpoint. It adds conservative remote Queue delete support for regular files, symlinks, and empty folders, but does not add rename, sync, resume, recursive non-empty folder delete, parallel transfers, or SFTP traffic monitor integration. `dev 0.1.8.3.1` is a small SFTP logging polish checkpoint on top of the file-transfer diagnostics baseline. `dev 0.1.8.3` is a file-transfer diagnostics checkpoint. Diagnostic logging is optional and OFF by default; when enabled, logs are intended to contain transfer metadata, queue decisions, paths, sizes, durations, and errors only. Logs must not contain passwords, private keys, secret values, terminal input/output, clipboard contents, or file contents. `dev 0.1.8.2` is a file manager safety polish checkpoint for the accepted File Manager baseline. The current file-transfer feature set includes single-file upload/download, sequential queue execution, Retry selected, Overwrite all / Skip all, experimental recursive folder upload/download through queue expansion, and conservative remote Queue delete. It is intentionally not a complete file transfer tool yet: parallel transfer, resume, sync/mirror, rename, chmod, permission/timestamp preservation, symlink following, and SFTP traffic monitor integration are not implemented.

Current experimental file-transfer limits:

- Folder upload/download exists, but it is still experimental and should be tested first with small non-production folders.
- Folder transfer works by scanning folders and expanding them into queue items; it is not a sync/mirror engine.
- Symlinks and special files are skipped, not followed.
- Permission and timestamp preservation are not implemented.
- Empty folders are represented through create-directory queue items, but edge cases should still be reported.
- Recursive non-empty folder delete, local delete, rename, chmod, advanced mkdir controls, and recursive folder transfer polish are still future work.

Not implemented yet:

- Recursive non-empty folder delete, local delete, rename, chmod, advanced mkdir controls, recursive folder transfer polish
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

The native Windows build and copied standalone deploy folder are confirmed to launch and run SSH/xterm workflows on Windows 10 and Windows 11. The current Windows work is still a deploy-folder workflow, not a full installer.

Known Windows alpha notes:

- Debug build startup is slower than Linux.
- First xterm.js terminal startup can take several seconds because Qt WebEngine initializes lazily.
- Subsequent terminal tabs are much faster.
- RAM usage can be hundreds of MB with an active xterm/WebEngine terminal because Qt WebEngine embeds Chromium components.
- Qt may create a cache folder under the DD-SSH AppData directory.
- The `windeployqt` deployment helper is validated for the current alpha workflow. A final installer is still future work.

## Known-host portability note

`dev 0.1.5.7` fixes the single-key known-host portability limitation found during Windows standalone testing. DD-SSH now accepts the legacy one-key format but saves known-host entries with a `keys` object so ED25519 and ECDSA host keys for the same `host:port` can coexist.

A true same-algorithm fingerprint mismatch is still treated as a strong host-key-changed warning.


## macOS-specific alpha notes

`dev 0.1.6.2` added the first Intel macOS `.app` / `.dmg` deployment path, and `dev 0.1.6.5` polished the DMG/dependency audit flow. It is useful for early testers, but it is not a final signed public macOS release.

Known macOS alpha notes:

- The first macOS package is x86_64 / Intel.
- Apple Silicon may run the Intel build through Rosetta 2; native arm64/universal builds are planned later.
- The DMG is unsigned and not notarized. Gatekeeper may require right-click → Open.
- macOS 12.x is not the primary target for the Qt 6.11.1 build; older macOS support may need a separate legacy Qt build experiment.
- Homebrew libssh/OpenSSL/zlib dependencies are bundled into the app during deployment, but tester machines should still be checked with `otool -L` if launch fails.


## SFTP traffic monitor

SFTP activity is not yet included in the live Session Traffic monitor. This is planned for a later transfer polish phase, not the first single-file download checkpoint.

- Overwrite dialogs do not yet compare existing and incoming file size/date metadata; this is planned as later transfer polish.

- If an upload is cancelled during transfer, a partial remote file may remain. Cancel cleanup is planned for later transfer polish.
