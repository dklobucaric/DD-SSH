# DD-SSH File Transfer Architecture

## dev 0.1.8.7 release/tester baseline [current]

The accepted File Manager baseline is queue-first: local `Queue upload` / `Delete local`, remote `Queue download` / `Delete remote`, sequential processing, metadata overwrite dialogs, Retry selected, destructive delete confirmation at the point where the queue reaches the delete item, and optional diagnostic logging. `dev 0.1.8.7` does not change this runtime architecture; it packages and documents it for external tester validation.


**Checkpoint:** dev 0.1.8.6 — Andromeda  
**Status:** Transfer queue foundation  
**Runtime behavior:** saved-session File Manager can browse local/remote directories, perform immediate one-file download/upload, queue multiple individual file downloads/uploads, and run queued items sequentially

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
- folder upload/download
- recursive transfer
- parallel transfer execution
- resume
- drag/drop
- folder transfer experiments
- diagnostic logging that never records file contents or secrets
- Session Traffic integration for SFTP bytes where practical

`dev 0.1.8.6.4` polishes File Manager wording and keeps the visible toolbar queue-first: local `Queue upload` / `Delete local`, remote `Queue download` / `Delete remote`. The legacy immediate helpers can remain internally but are no longer primary UI buttons. `dev 0.1.8.6` polishes File Manager delete UI on top of the accepted remote delete baseline. The local panel now offers `Delete local`, the remote panel offers `Delete remote`, and the saved-session menu says `Open File Manager`. Local/remote delete items are queued, destructive-confirmed, logged when diagnostics are enabled, and limited to regular files, symlinks, and empty directories; recursive non-empty folder delete is intentionally not implemented. `dev 0.1.8.3.1` polishes SFTP upload logs around overwrite checks. `dev 0.1.8.3` adds optional SFTP/file-transfer diagnostic logging around queue runs, immediate transfers, folder queue confirmations/summaries, retry-selected actions, overwrite/skip decisions, and item outcomes while preserving the existing transfer runtime. `dev 0.1.8.2` is a file manager safety polish checkpoint for the accepted file-manager/folder-transfer baseline; it updates documentation and in-app status text without changing the SFTP runtime. `dev 0.1.8.1.1` consolidates File Manager queue controls into two clearer panel actions while keeping the same folder-transfer engine: local `Queue upload` and remote `Queue download` accept selected files and folders. `dev 0.1.8.1` adds the first folder-transfer experiment by recursively scanning folders and expanding them into existing sequential queue items. Destination directories are represented as create-directory queue items; symlinks are skipped. `dev 0.1.8.0.4.1` stabilized the conservative transfer queue foundation before folder-transfer work: exit safety now reports running/pending queue work, queue/navigation controls are locked during a queue run, and no-pending-items feedback is clearer. `dev 0.1.8.0.3` polishes the first conservative transfer queue foundation with `Retry selected` for finished queue items. `dev 0.1.8.0.2` added Overwrite all / Skip all decisions for repeated overwrite conflicts. `dev 0.1.8.0` adds the queue foundation on top of the existing single-file upload/download paths. Users can queue multiple individual remote files for download and multiple individual local files for upload, then run the queue sequentially one item at a time. Existing immediate single-file actions remain available. The File Manager now includes an experimental recursive folder queue path and conservative local/remote delete, but it still does **not** implement parallel execution, resume, sync, recursive non-empty folder delete, rename, chmod, permission/timestamp preservation, symlink following, or OS-wide/global traffic monitoring yet.

---

## dev 0.1.8.0 transfer queue boundary

`dev 0.1.8.0` adds a queue wrapper around the existing single-file transfer operations:

- Remote `Queue download` adds selected remote files directly and asks for confirmation before recursively expanding selected remote folders into queue items
- Local `Queue upload` adds selected local files directly and asks for confirmation before recursively expanding selected local folders into queue items
- the queue table shows Status / Direction / Name / Size / Source / Target
- `Start queue` processes pending items sequentially, one file at a time
- queue item statuses are Pending / Running / Done / Failed / Cancelled / Skipped
- `Remove selected` removes selected non-running queue items
- `Clear finished` removes Done / Failed / Cancelled / Skipped items
- legacy immediate single-file transfer helpers may remain internally, but the primary visible File Manager workflow is queue-first
- queued transfers reuse the progress, speed, elapsed-time, completion, and cancel feedback from `dev 0.1.7.8`

`dev 0.1.7.8` previously polished transfer progress/cancel handling, `dev 0.1.7.7` added upload, `dev 0.1.7.6.1` polished download, and `dev 0.1.7.5` added the first two-panel File Manager foundation.

## Non-goals for dev 0.1.8.0

This checkpoint intentionally does not add:

- recursive folder upload/download
- delete
- rename
- chmod/chown
- folder upload/download
- recursive transfer
- parallel transfer execution
- resume
- drag/drop
- sync engine
- SFTP traffic monitor integration
- config schema migration
- terminal transport changes
- known-host behavior changes
- encryption/master-password changes

This is a queue-foundation checkpoint, not a full file-transfer implementation.

---

## dev 0.1.7.6 runtime two-panel browser with download

`dev 0.1.7.6` extends the two-panel File Manager with one remote-to-local download action:

```text
Saved session context menu
   -> Open File Manager
   -> left: local filesystem browser / download target
   -> right: remote SFTP browser
   -> select one remote file
   -> Download selected
   -> overwrite warning if needed
   -> basic progress dialog
   -> local panel refresh after success
```

The local panel uses Qt filesystem browsing and does not touch the SSH/SFTP stack except as the destination path. The remote panel continues to use the proven saved-session → known-host preflight → approved host-key verification → auth → libssh SFTP path. Folder transfer, recursive transfer, parallel transfer, resume, sync, and OS-wide/global traffic monitoring remains deferred.

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

### dev 0.1.7.6 — single-file download [passed]

- select remote file
- download to current local folder
- overwrite warning
- basic progress dialog
- refresh local panel after success

### dev 0.1.7.7 — single-file upload [passed]

- select local file
- upload to remote path
- overwrite warning
- progress/cancel if safe

### dev 0.1.7.8 — transfer progress/cancel polish [passed]


### dev 0.1.8.2 — file manager safety polish [done]

`dev 0.1.8.2` prepares the current File Manager and folder-transfer experiment for third-party tester builds. It adds tester checklist documentation, clearer known limitations, and alpha safety notes while preserving the existing transfer runtime from `dev 0.1.8.1.1`.

No SFTP engine, queue execution, folder scan, terminal runtime, known-host, Windows KEX, or config schema behavior is intentionally changed.

### dev 0.1.8.1.1 — queue selected UI consolidation [passed]

The File Manager now exposes two queue actions instead of four:

1. Local panel: `Queue upload`
2. Remote panel: `Queue download`

Selected regular files are queued directly. Selected folders still require recursive confirmation and are then expanded into the same queue item model introduced in `dev 0.1.8.1`. This keeps the UI simpler without changing the transfer engine.

### dev 0.1.8.1 — folder transfer experiment [passed/pending smoke]

Folder transfer deliberately reuses the queue model instead of adding a second transfer engine:

1. User selects one local or remote folder.
2. DD-SSH asks for explicit recursive-folder confirmation.
3. The folder is scanned.
4. Required destination directories are added as queue items.
5. Regular files are added as normal Upload/Download queue items.
6. Queue execution remains sequential and reuses overwrite, retry, cancel, and status handling.

Safety boundaries for this checkpoint:

- Symlinks and special files are skipped.
- Permission/timestamp preservation is not attempted.
- No sync/mirror/delete behavior exists.
- No parallel transfers or resume support.
- Folder scan has a conservative item limit to avoid accidentally queuing a whole filesystem.

### dev 0.1.8.0.4.1 — transfer queue stabilization polish [done]

`dev 0.1.8.0.4.1` deliberately avoids new transfer capabilities and tightens the existing queue foundation before recursive folder work. File Manager tabs now expose whether their queue has running or pending work so the main window can include that state in exit safety confirmation. During a queue run, queue controls, local/remote navigation controls, file panels, and the queue table are locked to avoid mid-transfer path changes or queue mutation.

The queue still processes one file at a time. Folder transfer, recursive traversal, parallel transfer, resume, sync, delete/rename/chmod/mkdir, and SFTP traffic monitor integration remain deferred.

### dev 0.1.8.0.3 — transfer queue retry-selected polish [passed]

`dev 0.1.8.0.3` keeps the queue sequential and one-file-at-a-time, but adds a safe retry path for selected finished items. `Done`, `Failed`, `Cancelled`, and `Skipped` queue items can be moved back to `Pending` with `Retry selected`, then processed again with `Start queue`. Existing overwrite/skip/overwrite-all decisions are reused when the retried item hits an existing target.

### dev 0.1.8.0.2 — transfer queue overwrite-all polish

`dev 0.1.8.0.2` keeps the queue sequential and one-file-at-a-time, but improves repeated overwrite conflicts. When queued downloads or uploads hit existing targets, the user can choose `Overwrite`, `Skip`, `Overwrite all`, `Skip all`, or `Cancel queue`. The all-decisions apply only to the same transfer direction during the current queue run.

### dev 0.1.8.0 — transfer queue foundation

- queue multiple individual remote files for download
- queue multiple individual local files for upload
- run queue sequentially, one item at a time
- show queue status table and finished/failed/cancelled/skipped states
- keep immediate one-file download/upload actions available

Further checkpoints should add folder transfer experiments, logging, traffic integration, and cross-platform stabilization.

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

`dev 0.1.8.9` adds SFTP transfer byte counters to the existing Session Traffic widget. Future traffic polish should still be added carefully:

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

### dev 0.1.7.7 — single-file upload foundation [passed]

### dev 0.1.7.8 — transfer progress/cancel polish [passed]

### dev 0.1.8.0.3 — transfer queue retry-selected polish [passed]

`dev 0.1.8.0.3` keeps the queue sequential and one-file-at-a-time, but adds a safe retry path for selected finished items. `Done`, `Failed`, `Cancelled`, and `Skipped` queue items can be moved back to `Pending` with `Retry selected`, then processed again with `Start queue`. Existing overwrite/skip/overwrite-all decisions are reused when the retried item hits an existing target.

### dev 0.1.8.0.2 — transfer queue overwrite-all polish

`dev 0.1.8.0.2` keeps the queue sequential and one-file-at-a-time, but improves repeated overwrite conflicts. When queued downloads or uploads hit existing targets, the user can choose `Overwrite`, `Skip`, `Overwrite all`, `Skip all`, or `Cancel queue`. The all-decisions apply only to the same transfer direction during the current queue run.

### dev 0.1.8.0 — transfer queue foundation

Queue foundation adds multiple individual file items and sequential execution while preserving existing transfer safety behavior. Overwrite metadata comparison, folder transfer, parallel transfer, and OS-wide/global traffic monitoring remains deferred.


### dev 0.1.8.6 — File Manager delete UI polish [accepted]

`dev 0.1.8.6` renames remote delete to `Delete remote`, adds local `Delete local`, and keeps both actions routed through the existing sequential transfer queue. Delete items require destructive confirmations before enqueue and before queue execution, and emit diagnostic log events when logging is enabled. Supported in this checkpoint: regular files, symlinks, and empty directories. Recursive non-empty folder delete is intentionally not implemented.

### dev 0.1.8.3.1 — SFTP logging polish [accepted]

`dev 0.1.8.3.1` clarifies upload logs around overwrite checks. Early upload calls are now logged as `SFTP upload preflight started`, while the real `SFTP file upload started` event is emitted only when the actual data-transfer phase begins after overwrite approval. This keeps tester logs easier to read without changing the transfer core.

### dev 0.1.8.3 — file transfer logging and diagnostics [accepted]

`dev 0.1.8.3` adds explicit logging around SFTP/File Manager workflows while keeping diagnostic logging optional and OFF by default. The goal is tester/debug visibility, not telemetry.

Logged when diagnostics are enabled:

- immediate upload/download completion, cancellation, and failure metadata
- transfer queue start/finish summaries
- queue item start/completion/failure/cancellation/skip metadata
- create local/remote directory queue item outcomes
- folder queue confirmation and folder selection summaries
- overwrite-all / skip-all / retry-selected decisions

Never log:

- passwords
- private key contents
- plaintext secret values
- terminal input/output
- clipboard contents
- file contents
- full `dd-ssh.json` content
