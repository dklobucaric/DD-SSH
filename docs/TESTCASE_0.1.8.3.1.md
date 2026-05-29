# Testcase — dev 0.1.8.3.1

Checkpoint: `dev 0.1.8.3.1` — SFTP logging polish.

## Preconditions

- A saved session with working SFTP.
- Diagnostic logging initially disabled.
- A small local test file.
- A remote destination folder where the same filename can be tested twice.

## Test steps

1. Start DD-SSH and confirm About shows `dev 0.1.8.3.1`.
2. Open Settings and confirm diagnostic logging is OFF by default.
3. Enable diagnostic logging.
4. Open Help → Open Log Folder.
5. Upload a local file to a remote folder where it does not yet exist.
6. Verify the log contains `SFTP upload preflight started` and then `SFTP file upload started` for the actual data transfer.
7. Upload/queue the same file again so the remote target already exists.
8. Choose Overwrite or Overwrite all.
9. Verify the log contains `SFTP upload target exists before transfer` before the overwrite decision.
10. Verify the real `SFTP file upload started` line appears only after overwrite approval.
11. Run a normal download and queue run to confirm existing SFTP logs still work.
12. Disable diagnostic logging and verify no new transfer events are appended.

## Security checks

Logs must not contain:

- passwords
- private-key contents
- plaintext secret values
- terminal input/output
- clipboard contents
- file contents
- full `dd-ssh.json`

Paths and filenames are accepted as diagnostic metadata because logging is opt-in.
