## dev 0.1.8.6.4 — Queue delete confirmation ordering polish

- Fixed queue confirmation ordering for destructive local/remote delete items.
- Delete confirmations are now shown just-in-time when the queue reaches the delete item instead of before the whole queue starts.
- This preserves visible queue order: earlier upload/download overwrite prompts appear before later delete confirmations.
- If a delete item is not confirmed, it stays Pending and the queue stops before that item.
- No SSH/SFTP transfer core, known-host, logging, config, or terminal paste behavior was changed.

## dev 0.1.8.6.3 — UI wording and File Manager polish

- Polished saved-session context menu labels: `Open terminal`, `Open file manager`, and `Open fallback shell`.
- Moved the About dialog Current phase line under Codename and added `Developer: DD-LAB vl. Dalibor Klobučarić`.
- Hid the legacy immediate remote-download toolbar button so the visible File Manager flow stays queue-first.
- Kept the accepted terminal paste newline fix from `dev 0.1.8.6.2`.
- No SSH/SFTP transfer core, known-host, logging, config, or packaging behavior was changed.

## dev 0.1.8.6.2 — Andromeda

Terminal paste newline safety polish.

- Fixed multiline paste into full-screen terminal applications such as `nano` by converting normalized clipboard line feeds (LF / `^J`) to terminal carriage returns (CR / `^M`) at the final PTY input boundary.
- This preserves YAML/config indentation and line structure when pasting through the toolbar button, right-click paste, Ctrl+Shift+V, or Command+V safe paste paths.
- Kept internal clipboard normalization and paste status counting unchanged.
- Kept terminal output rendering, SSH/SFTP/File Manager runtime, queue/delete behavior, diagnostic logging, known-host handling, Windows KEX workaround, and config schema unchanged.

## dev 0.1.8.6.1 — Andromeda

macOS/Linux release artifact version polish.

- Fixed `scripts/macos-deploy-release.sh` so DMG and dependency-audit filenames derive their default version from `DD_SSH_VERSION_STRING` in `CMakeLists.txt` instead of falling back to the stale `0.1.7.1` value.
- Kept the optional `DD_SSH_MACOS_VERSION=...` environment override for manual artifact naming.
- Fixed `scripts/linux-package-deb.sh` with the same automatic version detection so Debian package filenames/control metadata default to the current app checkpoint.
- Added package-version output lines to macOS and Debian packaging scripts so release logs clearly show the artifact version being produced.
- No runtime, SSH, SFTP, File Manager, queue, delete, logging, or config-schema behavior changes.

## dev 0.1.8.6 — Andromeda

File Manager delete UI polish.

- Renamed remote panel `Queue delete` to `Delete remote` while preserving queued destructive-confirmed behavior.
- Replaced the local panel immediate-upload toolbar button with `Delete local`.
- Added conservative local delete queue support for regular files, symlinks, and empty folders.
- Local delete requires enqueue confirmation and a second destructive confirmation before queue execution.
- Local delete is logged as WARN when diagnostic logging is enabled.
- Kept immediate upload helper code available internally, but removed it from the main local panel toolbar.
- Simplified saved-session context menu label to `Open File Manager`.
- Kept transfer core, queue model, remote delete behavior, logging baseline, overwrite metadata dialogs, terminal runtime, known-host handling, Windows KEX workaround, and config schema unchanged.

## dev 0.1.8.5 — Andromeda

Remote queue delete experiment.

- Added remote panel `Queue delete` action.
- Delete items are queued and executed through the existing sequential transfer queue.
- Supports regular remote files, symlinks, and empty remote directories.
- Recursive non-empty folder delete is intentionally not implemented in this checkpoint.
- Added destructive confirmation before queueing delete items and again before starting a queue that contains pending delete work.
- Added diagnostic logging for delete queue confirmation, queued delete items, delete start, success, and failure.
- Kept transfer core, upload/download, folder scan, overwrite metadata dialogs, terminal runtime, known-host handling, Windows KEX workaround, and config schema unchanged.

## dev 0.1.8.4 — Andromeda

Overwrite metadata dialog polish.

- Improved immediate download overwrite prompts with existing local file metadata and incoming remote file metadata.
- Improved immediate upload overwrite prompts with existing remote file metadata and incoming local file metadata.
- Improved queued download/upload overwrite prompts so Overwrite / Skip / Overwrite all / Skip all decisions show size and modified-time details where available.
- Preserved the accepted SFTP diagnostic logging baseline from `dev 0.1.8.3.1`.
- No transfer core, queue model, folder scan, SSH/terminal runtime, known-host, Windows KEX, or config schema changes.

## dev 0.1.8.3.1 — Andromeda

SFTP logging polish checkpoint.

### Improved
- Renamed the early upload log from `SFTP file upload started` to `SFTP upload preflight started` so overwrite checks do not look like a real data transfer already began.
- Added `allowOverwrite=true/false` to the upload preflight log for clearer queue diagnostics.
- Added an explicit `SFTP upload target exists before transfer` warning when the remote target already exists and overwrite approval is still needed.
- Moved the real `SFTP file upload started` log to the actual data-transfer phase, after overwrite approval and immediately before opening/truncating the remote file.

### Preserved
- No transfer core rewrite.
- No queue model changes.
- No folder scan engine changes.
- No SFTP traffic monitor integration yet.
- No terminal runtime, known-host, Windows KEX, config schema, or auth changes.

## dev 0.1.8.3 — Andromeda

File transfer logging and diagnostics checkpoint.

### Added / improved
- Added explicit SFTP/File Manager diagnostic logging around immediate single-file download/upload completion, cancellation, and failure.
- Added transfer queue logging for queue start, queue finish, item start, create-directory items, upload/download completion, failure, cancellation, skip, overwrite-all, skip-all, and retry-selected events.
- Added folder queue confirmation and folder selection summary logging so tester runs can be reconstructed from metadata when diagnostic logging is enabled.
- Added safe log-value formatting for File Manager log messages so newlines/tabs in paths are flattened before writing logs.

### Safety rules
- Diagnostic logging remains OFF by default and controlled from Settings.
- Logs contain transfer metadata only: paths, sizes/bytes, elapsed time, queue decisions, and error messages.
- Logs must not contain passwords, private keys, plaintext secret values, terminal input/output, clipboard contents, or file contents.

### Preserved
- No transfer core rewrite.
- No folder scan engine changes.
- No SFTP Session Traffic widget integration yet; that is planned after overwrite metadata/delete/monitor sequencing is settled.
- No terminal runtime, known-host, Windows KEX, or config schema changes.

## dev 0.1.8.2 — Andromeda

File manager safety polish checkpoint.

- Improved experimental folder queue confirmation with clearer safety rules and the queue item safety limit.
- Added folder queue scan summaries after recursive upload/download scans, including direct file items, folder file items, folder create items, skipped/cancelled items, and large-folder warnings.
- Added clearer warning text when a folder scan stops early because of a listing/permission error, recursion depth limit, or safety item limit.
- Improved queue failure messages for local destination/source path problems so the relevant path is shown.
- Refreshed local and remote file panels after a queue run completes.
- No transfer core, folder scan engine architecture, terminal runtime, known-host handling, Windows KEX workaround, config schema, upload/download flow, or queue status model changes are intended.

## dev 0.1.8.1.2 — Andromeda

Tester release polish checkpoint.

### Added / polished
- Added `docs/TESTER_CHECKLIST_0.1.8.1.2.md` for third-party tester flow, expected results, and bug-report format.
- Added `docs/BUILD_AND_TEST_0.1.8.1.2.md` and `docs/TESTCASE_0.1.8.1.2.md` for the tester-ready checkpoint.
- Updated tester-facing known limitations and File Transfer architecture notes to clearly mark folder transfer as experimental.
- Updated README, Test Matrix, Roadmap, documentation index, Welcome text, and About dialog to describe the current tester-release polish phase.

### Preserved
- Existing file manager runtime behavior from `dev 0.1.8.1.1`.
- Existing local `Queue upload` and remote `Queue download` buttons.
- Existing sequential queue, Retry selected, Overwrite all / Skip all, folder queue expansion, and exit safety behavior.
- No transfer core, folder scan engine, terminal runtime, known-host, Windows KEX, config schema, or SFTP traffic monitor changes.

## dev 0.1.8.1.1 — Andromeda

- Consolidated File Manager queue controls into two clearer panel actions: local `Queue upload` and remote `Queue download`.
- The consolidated queue buttons accept selected files and folders; files are queued directly, while folders still show the recursive-scan confirmation before being expanded into queue items.
- Removed the need for separate `Queue folder upload` / `Queue folder download` UI buttons without changing the underlying folder-transfer engine.
- Preserved sequential queue processing, Retry selected, Overwrite all / Skip all, queue exit safety, single-file immediate actions, and existing SFTP trust/auth behavior.
- No transfer core, terminal core, known-host, Windows KEX, sync, resume, parallel transfer, or SFTP traffic monitor changes.

## dev 0.1.8.1 — Andromeda

Folder transfer experiment.

- Added experimental recursive folder upload/download through the existing transfer queue.
- Added `Queue folder upload` for selected local folders.
- Added `Queue folder download` for selected remote folders.
- Folder scans expand into normal queue items rather than a separate transfer engine.
- Added queue item types for creating destination folders: `Create local dir` and `Create remote dir`.
- Symlinks and unsupported/special entries are skipped.
- Folder transfer remains conservative: no parallel transfers, resume, sync/mirror mode, permission/timestamp preservation, chmod, delete, rename, or SFTP traffic monitor integration.
- Existing single-file download/upload, queue, overwrite-all/skip-all, retry selected, and queue stabilization behavior remain in place.

## dev 0.1.8.0.4.1 — Andromeda

Compile hotfix for the transfer queue stabilization checkpoint.

### Fixed
- Fixed a compile-breaking multi-line `QStringLiteral` in `src/ui/SftpBrowserTab.cpp` that prevented `dev 0.1.8.0.4` from compiling.
- Queue stabilization behavior remains the same: exit safety for running/pending queue work, queue/navigation locking during queue runs, and clearer no-pending-items feedback.


Transfer queue stabilization polish checkpoint.

### Fixed / polished
- Exit safety now includes SFTP transfer queue work when a File Manager tab has a running queue or pending queue items.
- Exit confirmation warns that running/pending queue state is not persisted when closing DD-SSH.
- Queue controls, local/remote navigation controls, file panels, and queue table are locked while a queue run is active.
- `Start queue` gives a clearer message when there are no `Pending` queue items and suggests adding files or using `Retry selected`.

### Preserved
- Sequential one-at-a-time queue execution.
- Existing immediate single-file download/upload actions.
- Existing queue decisions: `Overwrite`, `Skip`, `Overwrite all`, `Skip all`, and `Cancel queue`.
- Existing `Retry selected` behavior for `Done`, `Failed`, `Cancelled`, and `Skipped` items.
- No folder transfer, parallel transfer, resume, sync, or SFTP traffic monitor integration yet.

### Documentation
- Added `docs/BUILD_AND_TEST_0.1.8.0.4.1.md` and `docs/TESTCASE_0.1.8.0.4.1.md`.

## dev 0.1.8.0.3 — Andromeda

Transfer queue retry-selected polish checkpoint.

### Added / polished
- Added `Retry selected` to the File Manager transfer queue controls.
- Selected `Done`, `Failed`, `Cancelled`, or `Skipped` queue items can be moved back to `Pending`.
- Requeued items keep their original direction, source path, target path, display name, and size.
- Existing `Start queue` then processes the requeued items through the same overwrite/skip/overwrite-all flow.
- `Pending` items are left unchanged and `Running` items cannot be retried while the queue is active.

### Preserved
- Sequential one-at-a-time queue execution.
- Existing immediate single-file download/upload actions.
- Existing queue overwrite decisions with `Overwrite`, `Skip`, `Overwrite all`, `Skip all`, and `Cancel queue`.
- No folder transfer, parallel transfer, resume, sync, or SFTP traffic monitor integration yet.

### Documentation
- Added `docs/BUILD_AND_TEST_0.1.8.0.3.md` and `docs/TESTCASE_0.1.8.0.3.md`.

## dev 0.1.8.0.2 — Andromeda

Transfer queue overwrite-all polish checkpoint.

### Added / polished
- Added queue overwrite decisions with `Overwrite`, `Skip`, `Overwrite all`, `Skip all`, and `Cancel queue`.
- `Overwrite all` applies to remaining queued conflicts of the same direction for the current queue run.
- `Skip all` skips remaining queued conflicts of the same direction for the current queue run.
- Queue download overwrite prompts remain before progress starts.
- Queue upload overwrite prompts remain visible after the temporary progress dialog closes.

### Preserved
- Sequential one-at-a-time queue execution.
- Existing immediate single-file download/upload actions.
- Existing progress, elapsed time, average speed, and cancel feedback.
- No folder transfer, parallel transfer, retry engine, sync, or SFTP traffic monitor integration yet.

### Documentation
- Added `docs/BUILD_AND_TEST_0.1.8.0.2.md` and `docs/TESTCASE_0.1.8.0.2.md`.

## dev 0.1.8.0.1 — Andromeda

Transfer queue bugfix polish checkpoint.

### Fixed
- Fixed queue overwrite prompts appearing underneath the queue progress dialog.
- Queue download overwrite decisions are now requested before the queue progress dialog is shown for that item.
- Queue upload overwrite prompts now close the temporary progress dialog before asking the user, then reopen progress only after overwrite is approved.
- Queue item cancel/continue prompts now appear after the progress dialog is closed.

### Preserved
- Sequential one-at-a-time queue execution.
- Existing immediate single-file download/upload actions.
- Existing progress, elapsed time, average speed, and cancel feedback.
- No folder transfer, parallel transfer, retry engine, sync, or SFTP traffic monitor integration yet.

### Documentation
- Added `docs/BUILD_AND_TEST_0.1.8.0.1.md` and `docs/TESTCASE_0.1.8.0.1.md`.

## dev 0.1.8.0 — Andromeda

Transfer queue foundation checkpoint.

### Added

- Added the first File Manager transfer queue foundation.
- Added `Queue download(s)` for multiple selected remote files.
- Added `Queue upload(s)` for multiple selected local files.
- Added a queue table with status, direction, name, size, source path, and target path.
- Added `Start queue`, `Remove selected`, and `Clear finished` controls.
- Queue processing runs one file at a time, intentionally avoiding parallel transfers for this foundation checkpoint.
- Queue items report `Pending`, `Running`, `Done`, `Failed`, `Cancelled`, and `Skipped` states.
- Existing one-file immediate download/upload actions remain available as `Download selected now` and `Upload selected now`.
- Queue download preserves safe local download behavior through the existing `QSaveFile`-based download path.
- Queue upload preserves the existing warning that cancelled uploads may leave a partial remote file.

### Preserved

- Existing single-file download/upload behavior remains available.
- Existing progress/speed/elapsed/cancel feedback remains available.
- Existing two-panel local/remote browsing remains available.
- Existing folder transfer blocking remains in place.
- Existing terminal/xterm.js runtime remains isolated from File Manager transfer work.
- Known-host multi-key support, Windows KEX workaround, diagnostic logging, terminal Session Traffic, config import/export, and native paste hardening should not regress.

### Not included

- No folder upload/download or recursive transfer.
- No parallel transfer execution.
- No resume.
- No drag/drop.
- No checksum comparison.
- No delete/rename/chmod/mkdir.
- No SFTP traffic integration in the live Session Traffic monitor yet.
- No overwrite metadata comparison dialog yet.

### Docs

- Added `docs/BUILD_AND_TEST_0.1.8.0.md` and `docs/TESTCASE_0.1.8.0.md`.

## dev 0.1.7.8 — Andromeda

Transfer progress and cancel polish checkpoint.

### Added / polished

- Transfer progress dialogs for both download and upload now show formatted progress details:
  - transferred size / total size
  - progress percent when total size is known
  - transfer speed
  - elapsed time
- Download completion dialog now includes elapsed time and average speed in addition to formatted size and raw byte count.
- Upload completion dialog now includes elapsed time and average speed in addition to formatted size and raw byte count.
- Download cancellation now shows a clear user-facing message and explains that the local target was not replaced because DD-SSH uses a safe temporary download file.
- Upload cancellation now shows a clear user-facing message and warns that a partial remote file may remain on the server.
- File Manager notice text now reflects single-file transfer progress/cancel polish.

### Preserved

- Existing single-file download behavior remains available.
- Existing single-file upload behavior remains available.
- Existing overwrite prompts and folder-transfer blocking remain unchanged.
- Existing two-panel local/remote browsing remains available.
- Existing terminal/xterm.js runtime remains isolated from the File Manager tab.
- Known-host multi-key support, Windows KEX workaround, diagnostic logging, Session Traffic for terminal tabs, config import/export, and native paste hardening should not regress.

### Not included

- No transfer queue.
- No folder upload/download.
- No delete/rename/chmod/mkdir.
- No SFTP traffic integration in the live Session Traffic monitor yet.
- No overwrite metadata comparison dialog yet.

### Docs

- Added `docs/BUILD_AND_TEST_0.1.7.8.md` and `docs/TESTCASE_0.1.7.8.md`.

## dev 0.1.7.7 — Andromeda

Single-file SFTP upload foundation checkpoint.

### Added

- Added the first local-to-remote transfer action in the File Manager: `Upload selected`.
- Selecting one local file uploads it into the currently open remote SFTP folder.
- Added overwrite warning when the remote target file already exists in the current listing.
- Added stale-listing safety: if the remote file exists but was not visible in the current browser listing, upload stops and asks the user to refresh before overwriting.
- Added a basic modal progress dialog with cancel support for the upload operation.
- Remote panel refreshes after successful upload.
- Upload uses the existing saved-session secret loading, SSH preflight, known-host decision flow, host-key verification before auth, libssh authentication, and SFTP subsystem path.

### Preserved

- Existing single-file download behavior remains available.
- Existing two-panel local/remote browsing remains available.
- Existing terminal/xterm.js runtime remains isolated from the File Manager tab.
- Known-host multi-key support, Windows KEX workaround, diagnostic logging, Session Traffic for terminal tabs, config import/export, and native paste hardening should not regress.

### Not included

- No folder upload or recursive transfer.
- No delete/rename/chmod/mkdir.
- No transfer queue or sync engine.
- No SFTP traffic integration in the live Session Traffic monitor yet.
- Overwrite dialog metadata comparison remains later transfer polish.

### Docs

- Added `docs/BUILD_AND_TEST_0.1.7.7.md` and `docs/TESTCASE_0.1.7.7.md`.

## dev 0.1.7.6.1 — Andromeda

Single-file SFTP download polish bugfix checkpoint.

### Fixed / polished

- Remote SFTP `Size` column sorting now uses the raw byte count instead of the formatted display text, so values like `94 B`, `77 KB`, and `90 MB` sort correctly by actual size.
- Download completion dialog now reports both the formatted size and raw byte count, for example: `Downloaded: 95 MB (99,614,720 bytes)`.
- Human-readable file sizes trim unnecessary `.0` decimals while preserving useful fractional values.

### Preserved

- Existing single-file download behavior remains unchanged.
- Existing overwrite prompt, folder-download blocking, progress dialog, local refresh, terminal runtime, known-host flow, Windows KEX workaround, and native paste paths should not regress.

### Deferred intentionally

- Overwrite dialog metadata comparison (existing/new size and modified date) remains later transfer polish.
- Upload, folder transfer, queue, sync, cancel polish, and SFTP traffic monitor integration remain out of scope.

### Docs

- Added `docs/BUILD_AND_TEST_0.1.7.6.1.md` and `docs/TESTCASE_0.1.7.6.1.md`.

## dev 0.1.7.6 — Andromeda

Single-file SFTP download foundation checkpoint.

### Added

- Added the first remote-to-local transfer action in the File Manager: `Download selected`.
- Selecting one remote file downloads it into the currently open local folder.
- Added overwrite warning when the local destination file already exists.
- Added a basic modal progress dialog with cancel support for the download operation.
- Local panel refreshes after successful download.
- Download uses the existing saved-session secret loading, SSH preflight, known-host decision flow, host-key verification before auth, libssh authentication, and SFTP subsystem path.
- Added `docs/BUILD_AND_TEST_0.1.7.6.md` and `docs/TESTCASE_0.1.7.6.md`.

### Preserved

- Existing two-panel local/remote browsing remains available.
- Existing terminal/xterm.js runtime remains isolated from the File Manager tab.
- Known-host multi-key support, Windows KEX workaround, diagnostic logging, Session Traffic for terminal tabs, config import/export, and native paste hardening should not regress.

### Not included

- No upload.
- No folder download or recursive transfer.
- No delete/rename/chmod/mkdir.
- No transfer queue or sync engine.
- No SFTP traffic integration in the live Session Traffic monitor yet.

## dev 0.1.7.4.1 — Andromeda

Read-only SFTP browser bugfix polish checkpoint.

### Fixed / polished

- Exit safety now includes open SFTP browser tabs in the close confirmation, alongside active SSH terminal sessions.
- Disabled alternating row colors in the SFTP browser table to avoid unreadable white rows on dark themes.
- Added stronger tab-bar scroll-button hints and right-side eliding for crowded tab bars, intended to improve macOS behavior when many tabs are open.
- Changed the browser `Up` button label to `↑ Up` so the action remains obvious even when native platform arrows are not rendered.
- Added the SFTP browser table to the application light/dark stylesheet coverage.

### Deferred intentionally

- SFTP byte counters are not yet included in the live Session Traffic monitor. This belongs with the upload/download transfer phase.
- `.` / `..` path normalization and hiding pseudo-directory entries remain later UI polish.
- No upload, download, delete, rename, local browser panel, queue, or progress/cancel behavior is added.
- No terminal transport, SSH trust-chain, known-host, Windows KEX workaround, or config schema behavior was intentionally changed.

### Docs

- Added `docs/BUILD_AND_TEST_0.1.7.4.1.md` and `docs/TESTCASE_0.1.7.4.1.md`.

## dev 0.1.7.4 — Andromeda

Read-only remote file browser checkpoint.

### Added

- Added `src/ui/SftpBrowserTab.h` and `src/ui/SftpBrowserTab.cpp` as the first graphical read-only remote SFTP browser tab.
- Saved-session context menu now offers `Open File Manager (read-only)`.
- The browser uses saved session data, existing known-host preflight, approved host-key verification before auth, and libssh SFTP directory listing.
- Browser UI includes a remote path field, `Go`, `Up`, `Refresh`, status label, and table columns for name, type, size, modified time, and permissions.
- Double-clicking a directory navigates into that directory. Double-clicking a regular file does not transfer or open it.
- Added `docs/BUILD_AND_TEST_0.1.7.4.md` and `docs/TESTCASE_0.1.7.4.md`.

### Preserved

- Existing terminal/xterm.js runtime remains isolated from the SFTP browser.
- Existing `SftpProbe` path remains the shared SFTP listing backend for this simple checkpoint.
- Known-host multi-key support, Windows KEX workaround, diagnostic logging, Session Traffic, config import/export, and native paste hardening should not regress.

### Not included

- No upload/download.
- No local file browser panel.
- No delete/rename/chmod/mkdir.
- No queue, progress/cancel transfer UI, recursive transfer, or sync engine.
- No encryption/master-password work.

## dev 0.1.7.3 — Andromeda

SFTP connection proof-of-concept checkpoint.

### Added

- Added `src/sftp/SftpProbe.h` and `src/sftp/SftpProbe.cpp` as the first libssh SFTP transport proof.
- Saved-session context menu now offers `Open File Manager (SFTP probe)`.
- The probe loads the saved session and referenced plain-v1 secret, runs SSH preflight, uses the existing known-host prompt flow, verifies the approved host key again before authentication, authenticates, initializes the SFTP subsystem, and lists the remote `.` directory into a read-only text tab.
- The result tab shows safe diagnostic information and a simple remote listing table with type, size, modified time, permissions, and name.
- Added `docs/BUILD_AND_TEST_0.1.7.3.md` and `docs/TESTCASE_0.1.7.3.md`.

### Not changed

- No graphical file browser is implemented yet.
- No upload, download, delete, rename, recursive transfer, transfer queue, progress dialog, or cancel workflow is implemented yet.
- No JSON schema migration was added.
- No terminal transport, xterm.js paste path, known-host data model, Windows KEX workaround, Session Traffic runtime, packaging scripts, or config import/export behavior was intentionally changed.
- Secrets and file contents are not written to diagnostic logs.

## dev 0.1.7.2 — Andromeda

File Transport architecture/design foundation checkpoint.

### Added

- Added `docs/FILE_TRANSFER_ARCHITECTURE.md` to document the planned libssh SFTP/File Manager design.
- Added a safe saved-session context-menu placeholder: `Open File Manager (planned)`.
- The placeholder explains the 0.1.7.x SFTP development track and does not open a network connection.
- Updated README, roadmap, test matrix, and checkpoint build/test docs for the File Transfer track.
- Added `docs/BUILD_AND_TEST_0.1.7.2.md` and `docs/TESTCASE_0.1.7.2.md`.

### Not changed

- No real SFTP subsystem is opened yet.
- No upload, download, file listing, delete, rename, queue, progress, or cancel behavior is implemented yet.
- No SSH trust-chain, known-host, JSON schema, terminal transport, paste handling, diagnostic logging engine, Session Traffic runtime, packaging script, or xterm.js asset behavior was changed.
- `dev 0.1.7.1` remains the closed terminal foundation baseline for regression testing.

## dev 0.1.7.1 — Andromeda

### Fixed
- Routed native xterm.js paste events through the same DD-SSH safe paste path used by the toolbar Paste button.
- Fixed right-click paste and Ctrl+Shift+V paste so bracketed paste wrappers such as `^[[200~` / `^[[201~` should not be sent as literal text to the remote shell.
- Added a defensive strip of bracketed-paste wrapper markers if they still reach the xterm.js data path.
- Kept keyboard Ctrl+C behavior unchanged so remote programs can still be interrupted from the focused terminal.

### Not changed
- No SSH trust-chain, known-host, JSON, logging, traffic monitor, packaging, or file-transfer behavior was changed.

### Docs
- Added `docs/BUILD_AND_TEST_0.1.7.1.md` and `docs/TESTCASE_0.1.7.1.md`.

## dev 0.1.7.0 — Andromeda

Terminal transport hardening checkpoint.

### Changed / hardened

- Changed SSH terminal output delivery from per-chunk `QString::fromUtf8()` conversion to a byte-stream path for xterm.js.
- `SshShellWorker` now emits raw output bytes as `QByteArray`.
- `TerminalBridge` forwards terminal bytes through Qt WebChannel as Base64 text.
- The WebEngine/xterm.js renderer decodes output with a streaming UTF-8 `TextDecoder`, reducing the risk of corrupt UTF-8 characters when multi-byte sequences are split across SSH reads.
- Changed terminal input queue storage from `QStringList` to queued UTF-8 bytes.
- Added partial-write-aware `ssh_channel_write()` handling so large paste/input is not silently truncated when libssh writes fewer bytes than requested.
- Basic/fallback terminal now decodes SSH output with a streaming Qt UTF-8 decoder before applying simple ANSI cleanup.

### Not changed

- No JSON schema changes.
- No changes to plain-v1 secrets.
- No changes to known-host trust rules.
- No file manager/SFTP implementation.
- No terminal input/output content is written to diagnostic logs.
- Session Traffic remains app SSH-channel byte counting, not global OS network monitoring.

### Docs

- Added `docs/TERMINAL_TRANSPORT.md`.
- Added `docs/BUILD_AND_TEST_0.1.7.0.md` and `docs/TESTCASE_0.1.7.0.md`.

## dev 0.1.6.9 — Andromeda

Bugfix stabilization checkpoint after the logging, Session Traffic, and config import/export safety work.

### Fixed / polished

- Renamed the xterm.js terminal toolbar button from `Ctrl+C` to `Copy` and made it copy selected terminal text to the clipboard. Keyboard Ctrl+C inside the terminal remains the way to send interrupt to remote programs.
- Renamed the BasicTerminal interrupt button from `Ctrl+C` to `Interrupt` so fallback/basic UI does not present a misleading copy-style label.
- Changed the config import preview confirmation action from generic `Yes` to `Import`.
- Changed the restore latest backup confirmation action from generic `Yes` to `Restore`.
- Added `docs/BUILD_AND_TEST_0.1.6.9.md` and `docs/TESTCASE_0.1.6.9.md`.

### Unchanged

- SSH trust-chain logic, known-host handling, JSON schema, plain-v1 secrets, diagnostic logging engine, Session Traffic counters, and packaging flow are intentionally unchanged.

## dev 0.1.6.8 — Andromeda

### Added
- Added config import/export safety previews before replacing or exporting `dd-ssh.json`.
- Preview shows file size, config version, session count, known-host count, trusted host-key count, secrets mode, saved secret counts, plaintext-secret presence, settings presence, metadata presence, and warnings.
- Added log summaries for config preview operations when diagnostic logging is enabled.
- Added `docs/CONFIG_IMPORT_EXPORT_SAFETY.md`, `docs/BUILD_AND_TEST_0.1.6.8.md`, and `docs/TESTCASE_0.1.6.8.md`.

### Security / safety
- Import now refuses invalid JSON/root-non-object files before the destructive replace confirmation.
- Export now warns when the active human-readable JSON may contain `plain-v1` plaintext secrets.
- Human-readable JSON and `plain-v1` compatibility remain unchanged.

### Not changed
- No SSH/auth/known-host runtime changes.
- No config schema migration.
- No encryption/master-password/keychain implementation.
- No Session Traffic or terminal transport changes.

## dev 0.1.6.7 — Andromeda

### Added

- Added a compact status-bar Session Traffic indicator for the active terminal tab.
- Shows live received/sent rates and total received/sent bytes for the active xterm.js or basic shell tab.
- The monitor follows the selected tab and shows a disconnected state with final totals after disconnect.
- Added SSH channel traffic counters in `SshShellWorker` for bytes read from and written to the shell channel.
- Added traffic lifecycle/summary logging when diagnostic logging is enabled:
  - `Traffic monitor started: session="..."`
  - `Session traffic summary: session="...", duration=..., received=..., sent=...`
  - `Traffic monitor stopped: session="...", received=..., sent=...`
- Added `docs/SESSION_TRAFFIC.md`, `docs/BUILD_AND_TEST_0.1.6.7.md`, and `docs/TESTCASE_0.1.6.7.md`.

### Scope / privacy

- This is application SSH shell-channel traffic, not global OS network traffic.
- Logs do not include terminal input, terminal output, passwords, private-key contents, clipboard contents, or full JSON config data.
- SFTP/file-transfer traffic is intentionally not implemented yet; the naming stays `Session Traffic` so future transport features can extend the same concept.

### Unchanged

- SSH trust-chain hardening remains unchanged.
- Human-readable `dd-ssh.json` and `plain-v1` secrets remain unchanged.
- Packaging scripts are unchanged except version defaults/comments.
- No SFTP/file-manager work is included in this checkpoint.

## dev 0.1.6.6 — Andromeda

### Added

- Added optional diagnostic logging controlled by Settings → Enable diagnostic logging.
- Added a cross-platform `AppLogger` foundation with INFO/WARN/ERROR lines in the format `2026-05-28 01:44:22.123 - INFO - App started`.
- Added standard per-user log folders:
  - Linux: `~/.local/state/DD-SSH/logs`
  - Windows: `%LOCALAPPDATA%\DD-SSH\logs`
  - macOS: `~/Library/Logs/DD-SSH`
- Added Help → Open Log Folder.
- Added a status-bar indicator when diagnostic logging is enabled.
- Added logging for app startup/shutdown, config/session loading, manual/saved connection requests, SSH handshake/authentication events, host-key verification before auth, shell lifecycle, disconnects, and key/auth errors.
- Added `docs/LOGGING.md`, `docs/BUILD_AND_TEST_0.1.6.6.md`, and `docs/TESTCASE_0.1.6.6.md`.

### Security / privacy

- Diagnostic logging is OFF by default.
- Logs intentionally do not include passwords, private-key contents, terminal input, terminal output, clipboard contents, or full `dd-ssh.json` data.

### Unchanged

- SSH runtime/trust-chain logic remains unchanged from `dev 0.1.6.3`.
- Human-readable `dd-ssh.json` and `plain-v1` secrets remain unchanged.
- Packaging scripts remain functionally unchanged except version defaults/comments.

# DD-SSH Changelog

## dev 0.1.8.6.4 — Queue delete confirmation ordering polish

- Renamed saved-session context menu actions:
  - `Open xterm.js terminal` → `Open terminal`
  - `Open File Manager` → `Open file manager`
  - `Open basic shell (fallback)` → `Open fallback shell`
- Updated Settings double-click description from `Open xterm.js terminal` to `Open terminal`.
- Reordered About dialog metadata so Current phase appears below Codename and above Milestone, while Developer appears in the old upper phase location.
- Added `Developer: DD-LAB vl. Dalibor Klobučarić` to the About dialog.
- Hid the legacy remote `Download selected now` toolbar button to keep the File Manager workflow queue-first.
- Swapped local toolbar order so `Queue upload` appears before `Delete local`.
- Kept the legacy immediate download/upload helper functions in code for now; they are no longer primary toolbar actions.

## dev 0.1.6.5 — Andromeda

macOS DMG and dependency polish.

- Updated the project identity to `dev 0.1.6.5`.
- Improved `scripts/macos-deploy-release.sh` with a generated `otool` dependency audit report beside the DMG.
- Added dependency warning output for leftover `/Users`, `/usr/local`, or `/opt/homebrew` paths after bundling.
- Added `STRICT_DEP_AUDIT=1` support so the Mac build machine can fail deployment when local/Homebrew paths remain unresolved.
- Added `README_FIRST.txt` inside the DMG staging folder with drag-to-Applications, Gatekeeper right-click Open, macOS 13+ guidance, Rosetta notes, and plain-v1 config warning.
- Kept SSH/session/terminal/config runtime behavior unchanged from `dev 0.1.6.3` and kept release-artifact rules from `dev 0.1.6.4` intact.

## dev 0.1.6.4 — Andromeda

Repo hygiene and release artifact workflow.

- Updated the project identity to `dev 0.1.6.4`.
- Added `.gitignore` protection for generated build folders, `dist/`, package artifacts, OS junk files, local logs, and IDE state.
- Added release-artifact documentation explaining that `.deb`, `.dmg`, `.zip`, AppImage/MSI/package outputs, and Windows deployment folders belong in GitHub Releases, not normal commits.
- Added SHA256 checksum helpers for Linux, macOS, and Windows:
  - `scripts/generate-checksums-linux.sh`
  - `scripts/generate-checksums-macos.sh`
  - `scripts/generate-checksums-windows.ps1`
  - `scripts/generate-checksums-windows.bat`
- Added a reusable release-notes template.
- Updated Linux/macOS packaging script defaults to `0.1.6.4`.
- Kept SSH/session/terminal/config runtime behavior unchanged from `dev 0.1.6.3`.

## dev 0.1.6.3 — Andromeda

Phase: SSH trust-chain hardening

- Updated the project identity to `dev 0.1.6.3`.
- Added a small `SshHostKeyExpectation` safety object used by authentication tests and real shell workers.
- Hardened saved-session xterm.js/basic shell startup so the real SSH shell connection verifies the approved host-key type and fingerprint after `ssh_connect()` and before password/private-key authentication.
- Hardened manual and saved-session authentication tests so the second/authentication connection also verifies the preflight-approved host key before attempting auth.
- Added auth-test output showing whether host-key verification was attempted and verified in the authentication connection.
- Preserved the existing human-readable `plain-v1` JSON format, known-host multi-key portability model, Windows KEX compatibility override, terminal behavior, and packaging scripts.
- This checkpoint intentionally avoids SFTP/file-manager work, traffic widgets, terminal byte-stream refactoring, JSON encryption, and large UI refactors.

## dev 0.1.6.2 — Andromeda

Phase: macOS Intel app/DMG foundation

- Updated the project identity to `dev 0.1.6.2`.
- Added `scripts/macos-build-release.sh` for a repeatable Intel macOS Release build using Qt 6.11.1, Homebrew libssh/OpenSSL/zlib, CMake, and Ninja.
- Added `scripts/macos-deploy-release.sh` for a deployable `DD-SSH.app` and unsigned `DD-SSH-0.1.6.2-macOS-x86_64.dmg`.
- Added a DMG staging layout with `DD-SSH.app` plus an `Applications` symlink for the standard drag-to-Applications macOS workflow.
- Added Homebrew dylib bundling/`install_name_tool` handling for `/usr/local` and `/opt/homebrew` dependencies discovered by `otool`.
- Added macOS build/deployment documentation and test notes for the first Intel native app bundle pass.
- Runtime SSH/session/terminal/config behavior is unchanged from `dev 0.1.6.1.1`.


## dev 0.1.6.1.1 — Andromeda

README screenshots and Debian packaging tutorial polish.

- Updated the project identity to `dev 0.1.6.1.1`.
- Added a README screenshot gallery using the Linux `.deb` validation screenshots.
- Added `docs/SCREENSHOTS.md` with descriptions for the Welcome screen, connected terminal, edit-session dialog, settings dialog, dark theme terminal, and About dialog.
- Added `docs/DEBIAN_PACKAGE_TUTORIAL.md` with a copy/paste workflow for building, inspecting, installing, testing, and removing the local `.deb`.
- Updated Linux packaging docs and test matrix to reflect that the first Debian package path was built, installed, launched, and visually validated.
- Updated the Debian package script default output version to `dd-ssh_0.1.6.1.1_amd64.deb`.
- Kept SSH/session/terminal/config runtime behavior unchanged in this documentation and packaging-polish checkpoint.

## dev 0.1.6.1 — Andromeda

First Debian package experiment.

- Updated the project identity to `dev 0.1.6.1`.
- Added Linux CMake install rules for the DD-SSH binary, desktop launcher, hicolor icons, README, license, and Markdown documentation.
- Added `scripts/linux-build-release.sh` for a repeatable Linux Release build.
- Added `scripts/linux-deploy-release.sh` for a local Linux release staging folder.
- Added `scripts/linux-package-deb.sh` to generate the first `.deb` package with `dpkg-deb`.
- Added Debian maintainer hooks to refresh desktop and icon caches when available.
- Added `docs/LINUX_PACKAGING.md` and refreshed packaging documentation for the 0.1.6.x packaging phase.
- Kept SSH/session/terminal/config runtime behavior unchanged in this packaging checkpoint.

## dev 0.1.5.9 — Andromeda

Stabilization docs and release polish.

- Updated the project identity to `dev 0.1.5.9`.
- Consolidated the successful Windows standalone deployment, known-host multi-key portability, and Windows libssh KEX compatibility results into the documentation set.
- Updated README, roadmap, test matrix, public alpha checklist, Windows build/deployment docs, troubleshooting, and known limitations to reflect the validated Windows 10 / Windows 11 / Linux status.
- Added a dedicated stabilization checkpoint report for the 0.1.5.6 → 0.1.5.8 validation sequence.
- Added a focused release checklist for future Andromeda public-alpha preparation.
- Kept SSH/session/terminal/config runtime behavior unchanged in this documentation checkpoint.

## dev 0.1.5.8 — Andromeda

Windows libssh handshake compatibility polish.

- Updated the project identity to `dev 0.1.5.8`.
- Added a Windows-only libssh key-exchange compatibility override before `ssh_connect()`.
- The Windows override limits libssh KEX negotiation to conservative algorithms already validated with Windows OpenSSH against OpenSSH 10 servers: `curve25519-sha256`, `curve25519-sha256@libssh.org`, `ecdh-sha2-nistp256`, `ecdh-sha2-nistp384`, `ecdh-sha2-nistp521`, and `diffie-hellman-group14-sha256`.
- Applied the compatibility override consistently to handshake tests, authentication tests, and real shell sessions.
- Added `DD_SSH_LIBSSH_DEBUG=1` as a local diagnostic switch for libssh protocol verbosity.
- Added `DD_SSH_DISABLE_WINDOWS_KEX_COMPAT=1` as a local escape hatch for comparing behavior with the compatibility override disabled.
- Added dedicated documentation for the real `lab.dd-lab.hr:2231` regression case where Windows libssh failed with `Failed to construct client init buffer` while Linux DD-SSH and Windows OpenSSH worked.

## dev 0.1.5.7 — Andromeda

Known-host multi-key portability polish.

- Updated the project identity to `dev 0.1.5.7`.
- Changed known-host storage from one key per `host:port` to multi-key storage under `keys`.
- Preserved compatibility with the old `algorithm` / `fingerprint` known-host format and migrates it on the next save.
- Added a separate **Additional SSH host key** decision path with **Trust additional key**, **Trust once**, and **Cancel**.
- Kept the strong **SSH host key changed** warning for true same-key-type fingerprint mismatches.
- Fixed the saved-session shell known-host decision flow so **Trust once** can continue the current shell-open attempt.
- Replaced `scripts/windows-deploy-release.bat` with the simpler working BAT validated during the Windows standalone deployment test.
- Added a dedicated known-host portability regression test note for the Windows 10 ECDSA vs Linux/Windows 11 ED25519 case.

## dev 0.1.5.6 — Andromeda

Windows standalone deployment test.

- Updated the project identity to `dev 0.1.5.6`.
- Hardened `scripts/windows-deploy-release.bat` for the standalone Windows release-folder test.
- Added deployment sanity checks for the deployed executable, Qt platform plugin, Qt WebEngine runtime hints, libssh, OpenSSL, and zlib DLLs.
- Updated Windows deployment documentation for `dist\windows-release`, no-manual-`PATH` launch testing, and clean Windows 10 validation.
- Updated README, About/Welcome text, roadmap, known limitations, public alpha checklist, build docs, packaging notes, and test matrix for the 0.1.5.6 focus.
- Kept SSH authentication, terminal runtime, config import/export/recovery, icon resources, and exit-safety behavior unchanged.

## dev 0.1.5.5 — Andromeda

Exit safety and user guide polish.

- Added application-level exit safety for active SSH terminal sessions.
- Closing the main window with active SSH connections now asks before disconnecting them.
- `File → Exit` now uses the same close protection as the window close button.
- Added user-facing documentation explaining that new sessions are saved only after successful SSH authentication.
- Updated README, user guide, troubleshooting, feature list, test matrix, and release-prep notes.
- Kept SSH authentication, xterm.js terminal runtime, config import/export, icons, and Windows deployment logic unchanged.

## dev 0.1.5.4 — Andromeda

Windows deployment experiment.

- Added `docs/WINDOWS_DEPLOYMENT.md` with the first `windeployqt`-based deployment workflow.
- Added `scripts/windows-deploy-release.bat` as an experimental helper for creating a standalone Windows release folder.
- Documented copying vcpkg runtime DLLs such as libssh/OpenSSL/zlib into the deployment folder.
- Documented testing the deployed app outside the build tree and without Qt/vcpkg PATH setup.
- Updated Windows build/release docs, roadmap, test matrix, and public-alpha checklist for deployment validation.
- Kept SSH/session/terminal runtime behavior and the updated icon resources unchanged.

## dev 0.1.5.3 — Andromeda

WebEngine startup polish.

- Added a visible terminal startup notice while the local xterm.js / Qt WebEngine renderer is preparing.
- Added clearer UI states for preparing/loading/ready/failed terminal renderer startup.
- Documented the Windows first-terminal startup delay as expected Qt WebEngine initialization behavior.
- Kept xterm.js terminal behavior, SSH/session logic, and icon resources otherwise unchanged.

## dev 0.1.5.2 — Andromeda

App icon integration.

- Added cross-platform icon resources generated from the uploaded DD-SSH master PNG.
- Added Qt resource icons under `:/icons/`.
- Set the Qt application/window icon from `:/icons/dd-ssh.png`.
- Added Windows `.ico` and `.rc` resources for the `.exe` icon.
- Added Linux PNG icon size variants for future desktop packaging.
- Added macOS `.iconset` and `.icns` prep for future app bundle packaging.
- Updated CMake integration for Windows and macOS icon resources.
- No SSH/session/terminal runtime logic changes are intended in this checkpoint.

## dev 0.1.5.1 — Andromeda

Windows build documentation and release build test preparation.

- Documented the first native Windows build path using MSVC x64, CMake, Ninja, Qt 6.11.1 MSVC 2022 64-bit, Qt WebEngine/WebChannel/Positioning, vcpkg `libssh`, and vcpkg `pkgconf`.
- Added `docs/WINDOWS_BUILD.md` with Debug and Release build commands.
- Documented Windows-specific observations: first WebEngine terminal startup delay, higher RAM usage from Qt WebEngine/Chromium, Qt cache folder, and AppData config path.
- Updated README, Welcome screen, Building docs, Test Matrix, Roadmap, and Public Alpha Checklist with Windows validation status.
- Added `.gitignore` coverage for Windows/MSVC local build artifacts.
- No SSH/session/terminal runtime logic changes are intended in this checkpoint.

## dev 0.1.5.0 — Andromeda

Public alpha release preparation.

- Added `docs/PUBLIC_ALPHA_CHECKLIST.md` as the final pre-alpha gate.
- Added `docs/RELEASE_NOTES_v0.2.0-alpha.md` draft.
- Added `docs/KNOWN_LIMITATIONS.md` for public tester visibility.
- Added GitHub issue templates for bug reports, terminal issues, config/recovery issues, and feature requests.
- Added pull request template with project safety checklist.
- Updated README, Welcome screen, and test matrix for public alpha preparation.

## dev 0.1.4.9 — Andromeda

**Focus:** Public alpha documentation pass.

- Rebuilt README as a public-alpha oriented project entry point.
- Added comprehensive documentation index.
- Added Getting Started guide.
- Added User Guide.
- Added Use Cases document.
- Added Features and Limitations document.
- Added Config Management document.
- Added Troubleshooting document.
- Expanded Architecture, Config Format, Security Notes, Roadmap, Building, Packaging, Release Process, and Test Matrix docs.
- Updated Welcome/About phase text to documentation pass.
- No SSH/session/terminal runtime logic changes are intended in this checkpoint.

## dev 0.1.4.8 — Andromeda

**Focus:** Config import/export/restore polish.

- Added File-menu config actions: Open Config Folder, Export Config, Import Config, Restore Latest Backup, and Exit.
- Export Config copies the active `dd-ssh.json` to a user-selected path.
- Import Config validates the selected JSON file, warns before replacement, creates a pre-import backup, then reloads settings and saved sessions.
- Restore Latest Backup restores the newest valid `dd-ssh.json.bak-*` backup and moves the previous active config aside as `dd-ssh.json.pre-restore-*`.

## dev 0.1.4.7 — Andromeda

**Focus:** Quick toolbar visibility polish.

- Hides quick action toolbar by default.
- Adds Settings option to show/hide quick action toolbar.
- Persists toolbar setting under `settings.behavior.show_quick_toolbar`.

## dev 0.1.4.6 — Andromeda

**Focus:** Session menu and dialog mode polish.

- File menu reserved for app/config actions.
- Session menu owns New Session, Connect / Auth test, and Edit selected session.
- ConnectDialog has Manual Connect, New Saved Session, and Edit Saved Session modes.

## dev 0.1.4.5.1 — Andromeda

**Focus:** Config recovery actions.

- Added Continue read-only.
- Added Restore latest valid backup.
- Added Create fresh config.
- Corrupt config files are preserved as `.corrupt-*`.

## dev 0.1.4.5 — Andromeda

**Focus:** Config recovery guard.

- Invalid/non-object `dd-ssh.json` is not overwritten automatically.
- Startup recovery warning lists backups and can open config folder.

## dev 0.1.4.4 — Andromeda

**Focus:** App theme foundation.

- Added app appearance setting: System / Light / Dark.
- Theme applies to Qt app chrome, not xterm.js terminal.

## dev 0.1.4.3 — Andromeda

**Focus:** Codename roadmap alignment.

- Reframed 0.1.x as Andromeda / MF 0.2 candidate line.
- Kept Launchpad as 0.0.x early-prototype history.

## dev 0.1.4.2 — Andromeda

**Focus:** Config path and backup policy implementation.

- Linux path casing changed to `DD-LAB/DD-SSH`.
- Implemented rotating config backups before saves.

## dev 0.1.4.1 — Andromeda

**Focus:** Settings dialog sizing polish.

- Settings dialog opens at a readable default size.

## dev 0.1.4.0 — Andromeda

**Focus:** Settings foundation.

- Added Settings dialog.
- Added terminal font settings.
- Added config path display.
- Added backup policy settings.
- Added plaintext secrets warning.

## dev 0.1.3.9 — Andromeda

**Focus:** Andromeda test matrix documentation.

- Added test matrix for Real Terminal Foundation validation.

## dev 0.1.3.8 — Andromeda

**Focus:** Terminal UI and status cleanup.

- Improved terminal header/state labels.
- Shortened terminal action labels.
- Improved status feedback.

## dev 0.1.3.7 — Andromeda

**Focus:** Reconnect disconnected terminal.

- Added Reconnect action for disconnected xterm.js terminal tabs.

## dev 0.1.3.6 — Andromeda

**Focus:** Terminal lifecycle polish.

- Added close confirmation for active SSH tabs.
- Added connected/disconnected tab markers.
- Improved remote disconnect/reboot handling.

## dev 0.1.3.5 — Andromeda

**Focus:** Welcome and changelog polish.

- Updated Welcome screen.
- Added codename roadmap.

## dev 0.1.3.4 — Andromeda

**Focus:** Double-click opens terminal.

- Saved session double-click opens xterm.js terminal by default.
- Auth test remains in context menu.

## dev 0.1.3.3 — Andromeda

**Focus:** Terminal compatibility checkpoint.

- Added codename/milestone display.
- Added reset local terminal action.
- Confirmed terminal app compatibility direction.

## dev 0.1.3.2.2 — Andromeda

**Focus:** Local xterm resource path fix.

- Fixed Qt resource path so bundled xterm.js/FitAddon load locally.

## dev 0.1.3.1 — Andromeda

**Focus:** xterm fit + SSH PTY resize.

- Added FitAddon.
- Added terminal resize reporting to SSH worker.
- Remote `stty size` follows window size.

## dev 0.1.3.0 — Andromeda

**Focus:** First xterm.js terminal renderer.

- Added first xterm.js renderer path through Qt WebEngine.

## dev 0.1.2.x — Launchpad-to-Andromeda bridge

- Added basic saved-session SSH shell channel.
- Added web terminal fallback.
- Added paste/input dispatch fixes.
- Added focus polish.

## dev 0.1.1.x — Launchpad

- Added saved sessions.
- Added plaintext secrets.
- Added connect from saved session.
- Added edit/delete session.
- Added duplicate target warning.

## dev 0.1.0.x — Launchpad

- Initial Qt GUI skeleton.
- Manual connection dialog.
- SSH handshake/auth tests.
- known_hosts storage.

## Codename roadmap

```text
0.0.x — Launchpad / early prototype history
0.1.x — Andromeda / current MF 0.2 candidate line
0.2.x — Orion
0.3.x — Vega
0.4.x — Cassiopeia
1.0.x — Apollo
```
