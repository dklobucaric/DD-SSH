# DD-SSH Third-Party Tester Checklist — dev 0.1.8.1.2

Checkpoint: `dev 0.1.8.1.2` — Tester release polish.

Thank you for testing DD-SSH. This is an alpha development build. It is meant for practical validation, not production-critical file movement yet.

## Before testing

- Use a test server or a safe test directory first.
- Do not begin with `/`, `/home`, `C:\`, `/Users`, or large backup directories.
- Create a small test folder with a few files and one nested subfolder.
- Keep backups of anything important.
- Remember that saved passwords/private keys may be stored in plaintext in `dd-ssh.json`.

Recommended first test folder:

```text
test-folder/
  a.txt
  b.txt
  sub/
    c.txt
```

## Basic application test

- [ ] App launches.
- [ ] About shows `dev 0.1.8.1.2`.
- [ ] A saved SSH terminal session opens.
- [ ] Terminal input/output works.
- [ ] Paste works.
- [ ] Ctrl+C interrupts the remote command.
- [ ] Closing the app with open tabs shows an exit confirmation.

## File Manager test

- [ ] Open File Manager from a saved session.
- [ ] Local panel lists local files.
- [ ] Remote panel lists remote SFTP files.
- [ ] `Queue upload` is visible on the local side.
- [ ] `Queue download` is visible on the remote side.
- [ ] Refresh and Up work on both panels.

## Single-file transfer test

- [ ] Download one small remote file.
- [ ] Upload one small local file.
- [ ] Try overwrite and confirm the overwrite prompt appears.
- [ ] Try Cancel during a test transfer and verify the message is clear.

## Queue test

- [ ] Queue several remote files for download.
- [ ] Queue several local files for upload.
- [ ] Start queue.
- [ ] Queue processes one item at a time.
- [ ] Done/Failed/Cancelled/Skipped statuses are understandable.
- [ ] Retry selected moves finished/skipped/cancelled items back to Pending.
- [ ] Overwrite all and Skip all work when several target files already exist.

## Folder transfer test

Folder transfer is experimental. Test only small folders first.

- [ ] Select a small local folder and click `Queue upload`.
- [ ] Confirm recursive queueing.
- [ ] Start queue and verify the folder structure appears remotely.
- [ ] Select a small remote folder and click `Queue download`.
- [ ] Confirm recursive queueing.
- [ ] Start queue and verify the folder structure appears locally.
- [ ] Symlinks/special files should be skipped, not followed.

## Things not expected to work yet

- Sync/mirror mode.
- Parallel transfers.
- Resume partial transfers.
- Preserve timestamps/permissions.
- SFTP traffic in the status-bar Session Traffic monitor.
- Delete, rename, chmod, advanced mkdir controls.
- Encrypted config/master password.

## Useful bug report format

```text
OS/version:
DD-SSH version:
Build type/package:
Server OS/OpenSSH version if known:
Auth type: password/private key
Feature tested:
Steps to reproduce:
Expected result:
Actual result:
Screenshots/log excerpt:
Can reproduce: yes/no
```

If diagnostic logging is enabled, include only relevant log excerpts. Never share passwords, private keys, full `dd-ssh.json`, or private file contents.
