# Build and Test — dev 0.1.7.2

**Checkpoint:** dev 0.1.7.2 — Andromeda  
**Focus:** File Transport architecture/design foundation  
**Runtime scope:** no real SFTP connection yet

---

## Goal

Validate that the small 0.1.7.2 checkpoint builds and that the existing terminal baseline remains stable while the File Manager placeholder and architecture docs are added.

This checkpoint should be boring on purpose.

No upload, download, remote listing, or SFTP subsystem should run in this version.

---

## Linux build

From the repository root:

```bash
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

Expected:

```text
DD-SSH starts normally.
About dialog shows dev 0.1.7.2.
Saved sessions still load.
```

---

## Linux smoke test

```text
[ ] App starts
[ ] About shows dev 0.1.7.2
[ ] Welcome tab mentions File Transfer architecture foundation
[ ] Saved sessions appear in the sidebar
[ ] Double-click saved session opens xterm.js terminal
[ ] Terminal commands work: whoami, pwd, clear, stty size
[ ] Toolbar Paste still works
[ ] Right-click paste still works
[ ] Ctrl+Shift+V still works
[ ] Ctrl+C interrupts a remote command
[ ] Session Traffic still updates during terminal activity
[ ] Diagnostic logging is still OFF by default
[ ] Help → Open Log Folder still works
[ ] Config import/export preview still works
[ ] Saved-session context menu shows Open File Manager (planned)
[ ] Open File Manager (planned) shows an info dialog only
[ ] Open File Manager (planned) does not create or modify config data
```

---

## Verify no SFTP runtime exists yet

The placeholder should not:

```text
[ ] open a remote connection
[ ] ask for trust/auth
[ ] list files
[ ] upload files
[ ] download files
[ ] create SFTP worker threads
[ ] change dd-ssh.json
```

If any of that happens in 0.1.7.2, it is a bug.

---

## Windows smoke test

After Linux passes and the change is pushed/pulled on Windows:

```powershell
cmake -S . -B build -G Ninja
cmake --build build
.\build\dd-ssh.exe
```

Minimum checks:

```text
[ ] App launches
[ ] About shows dev 0.1.7.2
[ ] Saved xterm.js terminal opens
[ ] File Manager placeholder dialog appears and does not crash
[ ] Windows KEX regression host still connects
[ ] Standalone deploy folder can still be generated after normal release workflow
```

---

## macOS smoke test

After Linux/Windows sanity checks:

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh.app/Contents/MacOS/dd-ssh
```

Minimum checks:

```text
[ ] App launches
[ ] About shows dev 0.1.7.2
[ ] Saved xterm.js terminal opens
[ ] Command+V paste still works
[ ] File Manager placeholder dialog appears and does not crash
[ ] DMG flow remains unaffected if packaging is tested
```

---

## Regression guard

Do not accept 0.1.7.2 if any of these regress:

```text
[ ] saved sessions fail to load
[ ] existing xterm.js terminal fails to open
[ ] paste handling regresses
[ ] Ctrl+C interrupt regresses
[ ] known-host multi-key behavior regresses
[ ] Windows-safe KEX workaround regresses
[ ] config import/export preview regresses
[ ] diagnostic logging starts logging secrets, terminal content, or clipboard content
```

---

## Expected result

0.1.7.2 should be a clean architecture checkpoint:

```text
Version updated.
Docs updated.
File Manager direction visible in UI.
No SFTP runtime yet.
Terminal baseline preserved.
```
