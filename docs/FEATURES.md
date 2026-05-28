# DD-SSH Features and Limitations

## Current feature set

### Core app

- Qt/C++ desktop app
- Main window with sidebar and terminal tabs
- Menu-driven UI
- Optional quick action toolbar
- About dialog with version/codename/milestone/config path

### Session management

- Saved sessions in `dd-ssh.json`
- Create saved session
- Edit saved session
- Delete saved session
- Duplicate target warning for same `username + host + port`
- Sidebar context menu
- Double-click opens xterm.js terminal
- Read-only File Manager / SFTP browser action for the future SFTP development track

### SSH/auth

- libssh integration
- SSH handshake
- Server banner display in auth tests
- Host key fingerprint display
- known_hosts trust flow
- Password authentication
- Private-key authentication

### Terminal

- xterm.js through Qt WebEngine
- Local bundled xterm.js assets
- FitAddon
- SSH PTY resize sync
- Direct keyboard input
- Paste
- Ctrl+C
- Reconnect after disconnect
- App exit warning when active SSH terminal sessions are still connected
- Tested full-screen apps: `htop`, `nano`, `vim`, `top`

### File transfer / File Manager

- Architecture documentation exists in `docs/FILE_TRANSFER_ARCHITECTURE.md`
- Saved-session context menu includes `Open File Manager (transfer enabled)`
- The File Manager uses saved session data, existing known-host preflight, approved host-key verification before auth, libssh SFTP initialization, and remote directory listing
- Left panel browses local files as the current download destination and upload source using Qt filesystem APIs
- Right panel browses remote SFTP directories with path, `Go`, `↑ Up`, `Refresh`, and name/type/size/modified/permissions columns
- Double-clicking directories navigates into them
- `Download selected` downloads one selected remote file into the currently open local folder
- `Upload selected` uploads one selected local file into the currently open remote folder
- Overwrite warning and basic progress dialog are included for single-file download and upload
- Delete, rename, folder transfer, queue, sync, and SFTP traffic monitor integration are not implemented yet

### Config

- Single JSON config file
- Linux default path: `~/.config/DD-LAB/DD-SSH/dd-ssh.json`
- Settings block
- sessions block
- known_hosts block
- secrets block
- metadata block
- Rotating backups
- Import/export
- New sessions are saved only after successful authentication
- Restore latest backup
- Corrupt config recovery

### Settings

- App theme: System / Light / Dark
- Terminal font family
- Terminal font size
- Config backup enable/disable
- Max backup count
- Show/hide quick action toolbar

## Known limitations

### Security

- Passwords and private keys are stored in plaintext when saved.
- No master password yet.
- No encrypted secrets backend yet.
- No SSH agent support yet.

### Terminal

- xterm.js terminal theme customization is intentionally deferred.
- Existing terminal tabs do not live-update font settings yet.
- Terminal preferences are still basic.

### Platform support

- Linux is the primary development and regression-test platform.
- Native Windows build/runtime validation has started and is documented in `docs/WINDOWS_BUILD.md`.
- Windows app launch, SSH/xterm terminal, and `htop` were confirmed during the first MSVC/Qt/vcpkg test pass.
- Windows Release build metrics and deployment are still pending.
- macOS builds are intended but not validated yet.
- Packaging/installers are not ready.

### Features not implemented

- Multi-Exec
- Keep-alive settings
- Graphical file browser and real upload/download file transfer
- Split panes
- Portable mode next to binary
- Custom config path picker
- Encrypted config/secrets
- Import/export UI for individual sessions only

## Public alpha meaning

DD-SSH is suitable for controlled public testing when clearly labeled as alpha:

```text
Works for real SSH terminal usage.
Stores secrets in plaintext.
Use on trusted machines only.
Report bugs with OS/build/config details.
```

It is not yet a stable 1.0 daily-driver release.


## Public alpha support docs

The repository now includes public-alpha preparation documents:

- `docs/PUBLIC_ALPHA_CHECKLIST.md`
- `docs/RELEASE_NOTES_v0.2.0-alpha.md`
- `docs/KNOWN_LIMITATIONS.md`
- GitHub issue templates under `.github/ISSUE_TEMPLATE/`


## SFTP browser polish

`dev 0.1.7.8` polishes the single-file download/upload transfer dialogs with progress percent, transferred size, speed, elapsed time, completion summaries, and explicit cancel feedback. `dev 0.1.7.7` added the first single-file SFTP upload action. `dev 0.1.7.6.1` polished the first single-file SFTP download action by fixing remote size sorting and completion-size reporting. `dev 0.1.7.4.1` disabled alternating row colors for readable dark-theme tables, included SFTP browser tabs in app-exit safety confirmation, and hinted tab scrolling for crowded tab bars.
