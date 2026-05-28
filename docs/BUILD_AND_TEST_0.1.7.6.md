# Build and test — dev 0.1.7.6

Checkpoint: `dev 0.1.7.6 — Single-file SFTP download foundation`

This checkpoint adds the first narrow transfer action to the File Manager: selecting one remote file and downloading it into the currently open local folder.

Still intentionally out of scope:

- upload
- delete / rename / chmod / mkdir
- folder transfer
- transfer queue
- sync engine
- SFTP traffic monitor integration

## Linux build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Smoke test

- Help → About shows `dev 0.1.7.6`
- Saved-session context menu shows `Open File Manager (download enabled)`
- File Manager opens with local panel on the left and remote SFTP panel on the right
- Remote directory listing still works
- Local directory browsing still works
- Select one remote file and click `Download selected`
- Download goes into the current local folder
- If the local target already exists, DD-SSH shows an overwrite warning
- Progress dialog appears during download
- Local panel refreshes after successful download
- Downloading a remote folder is refused with a clear message
- Upload/delete/rename/folder transfer actions are still unavailable
- SSH terminal tabs still open normally
- Paste, Ctrl+C, and terminal traffic monitor still work

## Cross-platform smoke

Run the same file-manager smoke test on:

- Linux
- Windows standalone/dev build
- macOS `.app` / DMG flow if needed

Pay special attention to local target paths:

- Linux: `/home/...`
- Windows: `C:/Users/...` or `C:\Users\...`
- macOS: `/Users/...`
