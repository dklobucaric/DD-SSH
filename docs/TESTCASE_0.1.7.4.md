# Testcase — dev 0.1.7.4

## Goal

Validate the first graphical read-only SFTP remote file browser while preserving the already-tested terminal foundation.

## Test environment

Recommended first test:

- Linux build machine
- one saved session with password auth
- one saved session with private-key auth
- one server where SFTP is known to work
- optional one server where SFTP is disabled/misconfigured, to verify clean error handling

## Test steps

### 1. Version check

- [ ] Launch DD-SSH
- [ ] Help → About DD-SSH shows `dev 0.1.7.4`
- [ ] Welcome tab mentions read-only remote file browser

### 2. Open browser

- [ ] Right-click a saved session
- [ ] Context menu shows `Open File Manager (read-only)`
- [ ] Click it
- [ ] Known-host decision flow appears if required
- [ ] A new tab opens named like `<session> files`
- [ ] Browser shows path field, `Go`, `Up`, `Refresh`, status label, and table

### 3. Directory listing

- [ ] Initial remote path `.` loads successfully on a known-good SFTP server
- [ ] Table shows Name / Type / Size / Modified / Permissions
- [ ] Directory entries are visible
- [ ] File entries are visible
- [ ] Diagnostic logging, if enabled, logs summary events only and does not log file contents

### 4. Navigation

- [ ] Double-click a directory
- [ ] Browser navigates into that directory
- [ ] Click `Up`
- [ ] Browser navigates to parent path
- [ ] Type `/` or another known remote path and click `Go`
- [ ] Click `Refresh` and verify the table reloads

### 5. Read-only safety

- [ ] No upload button exists
- [ ] No download button exists
- [ ] No delete button exists
- [ ] No rename button exists
- [ ] Double-clicking a regular file does not transfer or open it
- [ ] `dd-ssh.json` is not modified by browsing, except normal known-host last_seen updates if applicable

### 6. Failure handling

On a server where SFTP is disabled or unavailable:

- [ ] Browser attempt fails cleanly
- [ ] Error mentions SFTP subsystem/listing failure
- [ ] App does not crash
- [ ] Existing terminal sessions remain usable

### 7. Terminal regression

- [ ] Existing xterm.js terminal opens normally
- [ ] Paste still works via toolbar Paste
- [ ] Paste still works via right-click
- [ ] Ctrl+Shift+V / Command+V still work
- [ ] Ctrl+C still interrupts remote command
- [ ] Session Traffic indicator still works for terminal tabs

## Pass criteria

`dev 0.1.7.4` passes when read-only SFTP browsing works on at least one SFTP-enabled server, directory navigation works, no write/transfer actions exist, and the terminal baseline from `dev 0.1.7.1` remains stable.
