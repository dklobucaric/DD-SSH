# Testcase — dev 0.1.7.6.1

Checkpoint: `dev 0.1.7.6.1 — Single-file SFTP download polish`

## Required checks

- [ ] Help → About DD-SSH shows `dev 0.1.7.6.1`
- [ ] Existing saved sessions load correctly
- [ ] Existing xterm.js terminal still connects
- [ ] Terminal paste paths still work
- [ ] Terminal Ctrl+C interrupt still works
- [ ] Session Traffic monitor still works for terminal tabs
- [ ] Right-click a saved session and choose `Open File Manager (download enabled)`
- [ ] Local panel opens normally
- [ ] Remote panel lists a known SFTP-enabled server
- [ ] Remote `Name` sorting still behaves normally
- [ ] Remote `Type` sorting still behaves normally
- [ ] Remote `Size` sorting uses actual byte count across mixed displayed units such as B / KB / MB / GB
- [ ] Selecting one remote file and clicking `Download selected` downloads it into the current local folder
- [ ] Existing local target shows overwrite warning
- [ ] Cancel on overwrite leaves the local file unchanged
- [ ] Progress dialog appears during download
- [ ] Download completion dialog shows `Downloaded: <formatted size> (<raw bytes> bytes)`
- [ ] Local panel refreshes after download
- [ ] Downloaded file size matches the remote file size
- [ ] Attempting to download a folder is refused clearly
- [ ] Upload/delete/rename/folder transfer/queue are not present yet
- [ ] Closing the app with File Manager tabs open shows exit safety confirmation

## Suggested checks for this bugfix

Use a directory containing files with visibly different units, for example `94 B`, `77 KB`, `90 MB`, and larger files. Click the remote `Size` header in both directions and verify the order follows actual size.

Download a file large enough to show a human-readable unit and confirm the final dialog uses a message like:

```text
Downloaded: 95 MB (99,614,720 bytes)
```

## Expected result

DD-SSH keeps the working single-file download behavior from `dev 0.1.7.6`, while remote size sorting and the completion-size message are clearer and correct.
