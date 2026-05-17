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
- Tested full-screen apps: `htop`, `nano`, `vim`, `top`

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

- Linux is the main tested platform so far.
- Windows and macOS builds are intended but not validated in the same depth yet.
- Packaging/installers are not ready.

### Features not implemented

- Multi-Exec
- Keep-alive settings
- SFTP
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
