# Testcase — dev 0.1.7.5

Checkpoint: `dev 0.1.7.5 — Local + remote read-only file manager foundation`

## Version / startup

- [ ] Help → About DD-SSH shows `dev 0.1.7.5`
- [ ] Existing saved sessions load in the sidebar
- [ ] Existing terminal tabs can still be opened normally

## File Manager open flow

- [ ] Right-click a saved session
- [ ] Choose `Open File Manager (read-only two-panel)`
- [ ] Known-host preflight behaves the same as the previous SFTP browser checkpoint
- [ ] If allowed, a File Manager tab opens
- [ ] Tab has local browser on the left and remote SFTP browser on the right

## Local panel

- [ ] Local panel starts at a valid local directory
- [ ] Local path field + `Go` opens a valid local directory
- [ ] Local `↑ Up` moves to the parent directory
- [ ] Local `Refresh` reloads the current directory
- [ ] Double-clicking a local folder enters it
- [ ] Double-clicking/selecting a local file does not upload or modify anything

## Remote panel

- [ ] Remote panel lists remote `.` on a known SFTP-enabled server
- [ ] Remote path field + `Go` opens a known remote directory such as `/` or `/tmp`
- [ ] Remote `↑ Up` moves to the parent path
- [ ] Remote `Refresh` reloads the current remote path
- [ ] Double-clicking a remote directory enters it
- [ ] Double-clicking/selecting a remote file does not download/open/modify anything

## Regression checks

- [ ] Closing the app with an open File Manager/SFTP tab shows exit confirmation
- [ ] Closing the app with SSH terminal + File Manager tabs lists both groups
- [ ] xterm.js terminal still opens and works
- [ ] Toolbar Paste works
- [ ] Right-click paste works
- [ ] Ctrl+Shift+V / Command+V paste works
- [ ] Ctrl+C remote interrupt still works
- [ ] Terminal Session Traffic still works for active terminal tabs
- [ ] SFTP traffic is not expected in the live monitor yet
- [ ] Diagnostic logging stays OFF by default

## Not expected yet

- [ ] No upload/download buttons
- [ ] No delete/rename/chmod/mkdir actions
- [ ] No transfer queue
- [ ] No transfer progress/cancel UI
