# Testcase — dev 0.1.7.6

Checkpoint: `dev 0.1.7.6 — Single-file SFTP download foundation`

## Required checks

- [ ] Help → About DD-SSH shows `dev 0.1.7.6`
- [ ] Existing saved sessions load correctly
- [ ] Existing xterm.js terminal still connects
- [ ] Terminal paste paths still work
- [ ] Terminal Ctrl+C interrupt still works
- [ ] Session Traffic monitor still works for terminal tabs
- [ ] Right-click a saved session
- [ ] Choose `Open File Manager (download enabled)`
- [ ] Local panel opens on the user's home folder
- [ ] Remote panel lists the saved session's remote directory
- [ ] Local `Go`, `↑ Up`, and `Refresh` work
- [ ] Remote `Go`, `↑ Up`, and `Refresh` work
- [ ] Double-clicking local folders navigates locally
- [ ] Double-clicking remote folders navigates remotely
- [ ] Selecting a remote file and clicking `Download selected` downloads it into the current local folder
- [ ] Existing local target shows overwrite warning
- [ ] Cancel on overwrite leaves the local file unchanged
- [ ] Progress dialog appears during download
- [ ] Local panel refreshes after download
- [ ] Downloaded file size matches the remote file size
- [ ] Attempting to download a folder is refused clearly
- [ ] Upload/delete/rename/folder transfer/queue are not present yet
- [ ] Closing the app with File Manager tabs open shows exit safety confirmation

## Suggested files

Start with tiny text-like files such as `.profile`, `.bashrc`, `.wget-hsts`, or a test file created specifically for this purpose.

Then test one medium file if available, but avoid huge production backups until the transfer path is validated on all three platforms.

## Expected result

DD-SSH can perform one safe remote-to-local file transfer using the existing saved-session, known-host, host-key verification, authentication, and SFTP flow. The terminal foundation remains unchanged.
