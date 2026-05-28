# DD-SSH File Transfer Architecture

**Checkpoint:** dev 0.1.7.7 — Andromeda  
**Status:** Single-file SFTP upload foundation  
**Runtime behavior:** saved-session File Manager can browse local/remote directories, download one selected remote file, and upload one selected local file

---

## Goal

DD-SSH is moving from the closed terminal-foundation baseline into the File Transfer / File Manager development track.

The goal is to add a practical, safe, cross-platform file manager that can use the same saved session profiles as the terminal, while keeping the existing SSH terminal foundation stable.

The future File Manager should eventually provide:

- local file panel
- remote file panel
- remote directory browsing
- single-file download
- single-file upload
- overwrite warnings
- progress and cancel handling
- transfer queue
- folder transfer experiments
- diagnostic logging that never records file contents or secrets
- Session Traffic integration for SFTP bytes where practical

`dev 0.1.7.7` adds the first deliberately narrow local-to-remote transfer path: upload one selected local file into the currently open remote folder. `dev 0.1.7.6.1` preserved and polished the remote-to-local download path. Both transfer directions preserve the existing saved-session, known-host, host-key verification, auth, and libssh SFTP flow. The File Manager still does **not** implement folder transfer, queue, sync, delete, rename, chmod/mkdir, or SFTP traffic monitor integration yet.

---

## dev 0.1.7.7 single-file upload boundary

`dev 0.1.7.7` adds `Upload selected` to the local panel. The user selects one local file, confirms remote overwrite if needed, sees a basic progress dialog, and the remote panel refreshes after success. The right panel remains the remote SFTP destination selector.

`dev 0.1.7.6.1` previously polished the first download path, and `dev 0.1.7.5` added the first two-panel File Manager foundation.

## Non-goals for dev 0.1.7.7

This checkpoint intentionally does not add:

- recursive folder upload/download
- delete
- rename
- chmod/chown
- transfer queue
- sync engine
- SFTP traffic monitor integration
- config schema migration
- terminal transport changes
- known-host behavior changes
- encryption/master-password changes

This is a first-download checkpoint, not a full file-transfer implementation.

---

## dev 0.1.7.6 runtime two-panel browser with download

`dev 0.1.7.6` extends the two-panel File Manager with one remote-to-local download action:

```text
Saved session context menu
   -> Open File Manager (transfer enabled)
   -> left: local filesystem browser / download target
   -> right: remote SFTP browser
   -> select one remote file
   -> Download selected
   -> overwrite warning if needed
   -> basic progress dialog
   -> local panel refresh after success
```

The local panel uses Qt filesystem browsing and does not touch the SSH/SFTP stack except as the destination path. The remote panel continues to use the proven saved-session → known-host preflight → approved host-key verification → auth → libssh SFTP path. Upload, folder transfer, queue, sync, and SFTP traffic integration remain deferred.

## dev 0.1.7.4 runtime browser

`dev 0.1.7.4` adds a deliberately small saved-session read-only SFTP browser:

```text
Saved session context menu
   -> Open File Manager (read-only)
   -> load saved SessionProfile and referenced plain-v1 secret
   -> run SSH preflight handshake
   -> use existing known-host prompt/decision flow
   -> verify the approved host key again on the real SFTP connection before authentication
   -> authenticate using password or embedded private key
   -> initialize libssh SFTP subsystem
   -> list remote directory into a table
   -> allow read-only navigation with path, Go, Up, Refresh, and double-click directory
```

This proves the first UI layer for remote browsing while keeping the terminal baseline isolated. The browser is intentionally blocking/simple for this checkpoint and should evolve into a proper worker-backed File Manager tab when transfers/progress/cancel become necessary.

It does not modify `dd-ssh.json` except when the existing known-host flow intentionally saves a newly trusted host key or additional host-key algorithm.

---

## Core rule: use libssh SFTP, not shell hacks

File transfer must use libssh's SFTP API.

Do **not** implement file transfer by parsing shell commands such as:

```text
ls
find
cat
base64
scp command output
sftp command output
```

Shell-command parsing is fragile across distributions, locales, shell aliases, filenames with spaces, Unicode filenames, permission errors, symlinks, and terminal escape sequences.

The planned file transport path should use APIs such as:

```text
sftp_new
sftp_init
sftp_opendir
sftp_readdir
sftp_open
sftp_read
sftp_write
sftp_close
sftp_closedir
sftp_free
```

This keeps file transfer separate from terminal rendering and avoids corrupting the working xterm.js path.

---

## Baseline preservation rule

`dev 0.1.7.1` is the closed terminal foundation/hardening baseline.

Future file-transfer checkpoints must not regress:

- saved-session loading
- password authentication
- private-key authentication
- known-host multi-key portability
- Windows-safe libssh KEX compatibility
- host-key verification before sending secrets
- xterm.js terminal startup
- PTY resize
- UTF-8 terminal output
- large paste handling
- native paste paths
- Ctrl+C remote interrupt behavior
- diagnostic logging privacy rules
- Session Traffic monitor behavior
- config import/export preview behavior

Any future checkpoint that touches SSH core or terminal transport must have a clear reason and a regression test.

---

## High-level design

A saved session is not only a terminal profile. It is a connection profile.

```text
SessionProfile
   |
   +-- Terminal tab
   |
   +-- File Manager tab
   |
   +-- future diagnostics / traffic views
```

The terminal and File Manager should be separate UI views. A terminal can stay open while a File Manager tab opens for the same saved session.

The File Manager should not steal or reuse the terminal shell channel.

---

## Connection model

The preferred design is one SSH/SFTP connection per File Manager tab.

```text
Saved SessionProfile
   |
   +-- ConfigManager loads session and referenced secret
   |
   +-- SSH preflight / known-host check
   |
   +-- real SSH connection verifies approved host key before auth
   |
   +-- SFTP subsystem opens on that SSH connection
   |
   +-- File Manager tab receives directory listings / transfer progress
```

This is intentionally separate from the terminal tab's shell connection.

Why separate connections?

- terminal lifecycle stays isolated
- SFTP lifecycle can have its own errors, reconnect, progress, and cancel logic
- easier debugging
- safer threading model
- cleaner future transfer queue
- no accidental terminal input/output mixing

---

## Known-host and auth rule

Future SFTP connections must use the same trust-chain rules as real terminal connections.

Before sending a password or private key, the SFTP connection path must verify that the remote host key is the approved key selected by the known-host flow.

The future SFTP worker must not skip:

- host-key display and fingerprint verification
- trusted/unknown/changed/additional-key decisions
- Windows-safe KEX override where needed
- approved host-key verification before auth

SFTP must not become a shortcut around terminal security.

---

## Proposed future source layout

The exact names can still evolve, but the intended direction is:

```text
src/sftp/SftpConnectionWorker.h
src/sftp/SftpConnectionWorker.cpp
src/sftp/SftpFileEntry.h
src/sftp/SftpTransferJob.h
src/sftp/SftpTransferResult.h

src/ui/FileManagerTab.h
src/ui/FileManagerTab.cpp
src/ui/RemoteFileModel.h
src/ui/RemoteFileModel.cpp
src/ui/LocalFileModel.h
src/ui/LocalFileModel.cpp
```

Suggested responsibilities:

### `SftpConnectionWorker`

- runs blocking libssh/SFTP work away from the UI thread
- opens SSH connection
- verifies known-host expectations
- authenticates
- initializes SFTP subsystem
- lists remote directories
- later performs upload/download
- emits safe result signals
- never logs secrets or file contents

### `SftpFileEntry`

Represents one remote directory entry:

```text
name
type: file / directory / symlink / other
size
modified time
permissions display string if practical
```

### `FileManagerTab`

- owns local/remote panel UI
- starts SFTP worker actions
- shows current local path and remote path
- handles refresh/go up/home/enter directory
- later handles upload/download buttons

### `RemoteFileModel`

- Qt model for remote entries
- supports sorting by name/type/size/modified
- displays errors without crashing the tab

### `LocalFileModel`

- can use Qt filesystem APIs or a thin model around them
- should handle platform differences cleanly

---

## First implementation sequence

### dev 0.1.7.2 — architecture/design foundation

- version/docs update
- File Manager placeholder action
- no SFTP runtime

### dev 0.1.7.3 — SFTP connection proof of concept

- use saved session
- perform known-host/auth path safely
- open SFTP subsystem
- list remote home/current directory to a simple debug dialog or log-safe output
- no two-panel browser yet

### dev 0.1.7.4.1 — read-only SFTP browser bugfix polish [passed]

### dev 0.1.7.5 — local + remote read-only file manager foundation [passed]

- app-exit safety includes open SFTP browser tabs
- alternating row colors disabled for SFTP table readability
- tab scroll-button hints for crowded tab bars
- no transfer/traffic/path-normalization expansion

### dev 0.1.7.4 — read-only remote file browser

- remote list panel
- name/type/size/modified/permissions
- editable remote path + Go
- enter folder by double-click
- go up
- refresh
- no upload/download/delete

### dev 0.1.7.5 — local + remote two-panel UI

- left local browser
- right remote browser
- read-only browsing

### dev 0.1.7.6 — single-file download [current]

- select remote file
- download to current local folder
- overwrite warning
- basic progress dialog
- refresh local panel after success

### dev 0.1.7.7 — single-file upload

- select local file
- upload to remote path
- overwrite warning
- progress/cancel if safe

Further checkpoints should add queueing, folder transfer experiments, logging, traffic integration, and cross-platform stabilization.

---

## UI direction

The early File Manager UI should stay simple:

```text
+--------------------------------------------------------+
| Local path                         | Remote path       |
|--------------------------------------------------------|
| local files                        | remote files      |
| local folders                      | remote folders    |
|                                    |                   |
| [Refresh] [Upload ->]              | [<- Download]     |
+--------------------------------------------------------+
```

No fancy sync engine, no cloud behavior, no complex commander clone in the first pass.

Start with read-only browsing, then one-file transfer.

---

## Logging and privacy rules

Diagnostic logging is OFF by default and remains opt-in.

Future file transfer logging may include:

- SFTP connection started/completed/failed
- directory listing started/completed/failed
- transfer started/completed/failed
- source/destination paths if acceptable for diagnostics
- file size
- duration
- error messages
- coarse byte totals

Future file transfer logging must never include:

- password values
- private key contents
- secret refs resolved to values
- terminal input/output
- clipboard content
- file contents
- full `dd-ssh.json`

If in doubt, log less.

---

## Session Traffic integration

The current Session Traffic monitor counts terminal shell-channel bytes for active terminal tabs.

Future SFTP traffic integration should be added carefully:

- show live transfer rates for active file manager / transfer tab if practical
- count application-level SFTP read/write bytes, not global OS traffic
- avoid noisy per-millisecond logging
- log lifecycle summaries only when diagnostic logging is enabled

Suggested future diagnostic entries:

```text
SFTP transfer started: session="doma", direction="download", size=123456
SFTP transfer completed: session="doma", received=123456, sent=0, duration_ms=420
SFTP transfer failed: session="doma", error="Permission denied"
```

No file contents should ever be logged.

---

## Cross-platform concerns

File transfer must be tested on:

- Linux
- Windows 10/11 standalone deploy folder
- macOS Intel `.app` / `.dmg`

Specific cases to test later:

- filenames with spaces
- Croatian/Unicode filenames: `čćžšđ ČĆŽŠĐ €`
- large files
- zero-byte files
- permission denied
- remote path not found
- disconnect during listing
- disconnect during transfer
- symlinks
- overwrite/cancel choices
- different remote home directories
- Windows local paths with drive letters
- macOS/Linux hidden files

---

## Current SFTP browser behavior

`dev 0.1.7.4` changes the saved-session context-menu action to `Open File Manager (read-only)`.

The browser:

- opens a real SSH/SFTP connection path using the saved session
- uses the existing known-host prompt and trust-chain flow
- verifies the approved host key again before authentication
- initializes the libssh SFTP subsystem
- lists remote directories into a read-only table
- supports path entry, Go, Up, Refresh, and double-click directory navigation
- does not upload, download, delete, rename, or transfer files
- does not modify `dd-ssh.json` except when the existing known-host flow intentionally saves trusted host-key data
- does not touch terminal tabs

This proves the first remote browser UI without risking the tested terminal baseline.

### dev 0.1.7.6.1 — single-file download polish [passed]

### dev 0.1.7.7 — single-file upload foundation [current]

Remote SFTP Size sorting uses raw byte counts instead of formatted display strings, and completion dialogs report formatted size plus raw bytes. Overwrite metadata comparison remains deferred.
