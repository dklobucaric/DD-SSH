# DD-SSH General Mindmap

## Center

```text
DD-SSH
Clean cross-platform SSH client and session manager
```

## Current identity

```text
Version: dev 0.1.5.7
Codename: Andromeda
Milestone: MF 0.2 candidate
Phase: Known-host multi-key portability polish
```

## Main branches

### Terminal

- xterm.js local renderer
- Qt WebEngine
- FitAddon
- PTY resize
- Ctrl+C
- Paste
- Reconnect
- htop/nano/vim/top/clear tested

### Sessions

- Saved sessions
- Create
- Edit
- Delete
- Duplicate target warning
- Double-click opens terminal
- Manual auth test remains available

### Config

- One `dd-ssh.json`
- Settings
- Sessions
- known_hosts
- Secrets
- Metadata
- Backups
- Import/export
- Recovery

### Security

- known_hosts trust checking
- Changed host key should block
- Plaintext secrets warning
- No secrets in logs
- Future encryption planned

### Settings

- App theme
- Terminal font
- Backup policy
- Quick toolbar visibility
- Config path display

### Future

- Public alpha release
- Config path / portable mode
- Keep-alive
- Multi-Exec
- Packaging
- Encrypted secrets
```
