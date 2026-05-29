# Testcase — dev 0.1.8.4

Checkpoint: `dev 0.1.8.4` — Overwrite metadata dialog polish.

## Preconditions

- A saved session with working SFTP.
- A local folder where a downloaded file can already exist.
- A remote folder where an uploaded file can already exist.
- At least one small test file.

## Test steps

1. Start DD-SSH and confirm About shows `dev 0.1.8.4`.
2. Download a remote file once.
3. Download the same remote file again using immediate `Download selected now`.
4. Confirm the overwrite dialog shows existing local file size/date and incoming remote file size/date.
5. Upload a local file once.
6. Upload the same local file again using immediate `Upload selected now`.
7. Confirm the overwrite dialog shows existing remote file size/date and incoming local file size/date.
8. Queue a download where the local target already exists.
9. Confirm the queue overwrite dialog shows existing local and incoming remote metadata.
10. Queue an upload where the remote target already exists.
11. Confirm the queue overwrite dialog shows existing remote and incoming local metadata.
12. Test Overwrite, Skip, Overwrite all, Skip all, and Cancel queue.
13. Confirm normal upload/download, folder queue, Retry selected, and diagnostic logging still work.

## Security checks

Overwrite dialogs may show paths, filenames, sizes, and modified timestamps. They must not expose passwords, private keys, plaintext secret values, file contents, terminal input/output, clipboard contents, or full `dd-ssh.json`.
