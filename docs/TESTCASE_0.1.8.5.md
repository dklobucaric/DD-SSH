# Testcase — dev 0.1.8.5

Checkpoint: `dev 0.1.8.5` — Remote queue delete experiment.

## Required checks

1. Start DD-SSH and confirm About shows `dev 0.1.8.5`.
2. Open File Manager for a test server/session.
3. Upload or create a harmless test file on the remote side.
4. Select that remote test file and click `Queue delete`.
5. Confirm the enqueue warning.
6. Verify a `Delete remote file` item appears in the queue.
7. Click `Start queue`.
8. Verify DD-SSH shows a second destructive confirmation before deleting.
9. Choose No/Cancel and confirm the item remains Pending.
10. Click `Start queue` again, confirm delete, and verify the remote test file disappears after refresh.
11. With diagnostic logging enabled, verify delete queued/started/completed events are logged.
12. Verify logs do not contain password, private key content, plaintext secret values, file contents, clipboard contents, or terminal input/output.
13. Try a non-empty remote folder if safe: it should fail safely because recursive delete is intentionally not implemented.
14. Confirm upload/download/queue/folder upload/download still work.
