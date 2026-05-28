# Build and Test — dev 0.1.7.3

**Checkpoint:** dev 0.1.7.3 — Andromeda  
**Milestone:** SFTP connection proof of concept  
**Goal:** prove that a saved DD-SSH session can open a libssh SFTP subsystem and list a remote directory without adding a graphical file browser or transfer actions yet.

---

## Scope

This checkpoint adds the first SFTP runtime proof:

- saved-session context menu action: `Open File Manager (SFTP probe)`
- saved session and referenced plain-v1 secret loading
- existing SSH preflight / known-host prompt flow
- approved host-key verification again on the real SFTP connection before authentication
- password or private-key authentication
- `sftp_new()` / `sftp_init()`
- `sftp_opendir()` / `sftp_readdir()` listing of remote `.`
- read-only text result tab

It does **not** add:

- graphical local/remote file browser
- upload
- download
- delete
- rename
- recursive transfers
- transfer queue
- progress/cancel UI
- config schema migration

---

## Linux build

From the project root:

```bash
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

Expected:

```text
Build completes.
App starts.
Help → About DD-SSH shows dev 0.1.7.3.
```

---

## Linux SFTP probe smoke test

Use at least one saved session that already works in the xterm.js terminal.

1. Start DD-SSH.
2. Confirm saved sessions load in the sidebar.
3. Right-click a saved session.
4. Choose `Open File Manager (SFTP probe)`.
5. Accept known-host decisions only if the fingerprint is expected.
6. Confirm a new read-only text tab opens.

Expected success output contains:

```text
DD-SSH SFTP connection proof of concept
Status: SUCCESS
SFTP subsystem initialized and remote directory listed successfully.
Remote listing:
```

The listing should show columns similar to:

```text
TYPE        SIZE          MODIFIED               PERMISSIONS  NAME
```

---

## Terminal regression smoke test

After running the SFTP probe, verify the terminal baseline still works:

```text
[ ] double-click saved session opens xterm.js terminal
[ ] whoami works
[ ] pwd works
[ ] clear works
[ ] htop/top/nano/vim still work where previously validated
[ ] toolbar Paste works
[ ] right-click paste works
[ ] Ctrl+Shift+V / Command+V works
[ ] Ctrl+C interrupts a remote process
[ ] Session Traffic status bar still updates for terminal tabs
```

---

## Security/logging checks

With diagnostic logging enabled, logs may include:

```text
SFTP probe started
SFTP probe authentication successful
SFTP probe successful: entries=<count>
```

Logs must **not** include:

```text
password values
private-key contents
terminal input/output
clipboard content
file contents
full dd-ssh.json
```

---

## Windows/macOS smoke test

For this checkpoint, Windows and macOS can use a shorter pass after Linux succeeds:

```text
[ ] app builds/launches
[ ] About shows dev 0.1.7.3
[ ] saved xterm.js terminal still opens
[ ] Open File Manager (SFTP probe) can initialize SFTP on at least one known-good saved session
[ ] probe result tab appears and does not crash the app
```

---

## Suggested commit

After Linux validation passes:

```bash
git add .
git commit -m "Add SFTP connection proof of concept for dev 0.1.7.3"
git push origin main
```
