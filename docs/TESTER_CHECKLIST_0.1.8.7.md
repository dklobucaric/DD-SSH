# Tester checklist — dev 0.1.8.7

Checkpoint: `dev 0.1.8.7` — Release/tester packaging polish.

Please test with harmless files and small test folders first. Do not point delete or folder-transfer tests at important production paths.

## 1. Install / launch

- [ ] Linux package / Windows portable / macOS DMG installs or launches.
- [ ] About shows `dev 0.1.8.7`.
- [ ] Welcome screen describes the terminal + File Manager alpha.
- [ ] App starts without requiring manual runtime PATH changes.

## 2. Terminal

- [ ] Saved SSH session opens a terminal.
- [ ] `htop`, `nano`, or similar fullscreen tool works.
- [ ] Paste a multiline YAML/config block into `nano`; formatting is preserved.
- [ ] Ctrl+C interrupts a remote command.
- [ ] Disconnect/reconnect path behaves cleanly.

## 3. File Manager browsing

- [ ] Right-click saved session -> Open file manager.
- [ ] Local panel lists local files.
- [ ] Remote panel lists SFTP files.
- [ ] Go / Up / Refresh work on both panels.

## 4. Queue upload/download

- [ ] Queue upload of one small local file.
- [ ] Queue download of one small remote file.
- [ ] Queue multiple files.
- [ ] Start queue processes one item at a time.
- [ ] Retry selected moves finished/skipped/cancelled/failed items back to Pending.
- [ ] Remove selected and Clear finished work.

## 5. Folder transfer experiment

Use a tiny folder first, for example:

```text
test-folder/
  a.txt
  b.txt
  sub/
    c.txt
```

- [ ] Queue upload of a small local folder.
- [ ] Queue download of a small remote folder.
- [ ] Folder scan summary appears.
- [ ] Created folders and files appear as queue items.
- [ ] Queue completion refreshes panels.

## 6. Overwrite and delete safety

- [ ] Overwrite dialog shows existing/incoming metadata.
- [ ] Overwrite all / Skip all work for repeated conflicts.
- [ ] Delete local queues a local delete item; it is not instant.
- [ ] Delete remote queues a remote delete item; it is not instant.
- [ ] Delete confirmation appears when queue reaches the delete item, preserving queue order.
- [ ] Non-empty recursive folder delete is not expected to work.

## 7. Diagnostic logging

- [ ] Logging is OFF by default.
- [ ] Enable logging in Settings.
- [ ] Help -> Open Log Folder works.
- [ ] Upload/download/folder/delete events are logged.
- [ ] Logs do not contain passwords, private keys, plaintext secrets, terminal input/output, clipboard contents, or file contents.
- [ ] Disable logging and confirm new transfer events stop being added.

## Bug report format

Please include:

```text
OS/version:
DD-SSH version from About:
Build type/package used:
Steps to reproduce:
Expected result:
Actual result:
Screenshots/log excerpt if useful:
Was diagnostic logging enabled? yes/no
```
