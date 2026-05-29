# Testcase — dev 0.1.8.3

Checkpoint: `dev 0.1.8.3` — File transfer logging and diagnostics.

## Required checks

- [ ] About dialog shows `dev 0.1.8.3`.
- [ ] Diagnostic logging is OFF by default.
- [ ] Enable diagnostic logging in Settings.
- [ ] Help → Open Log Folder opens the expected log folder.
- [ ] Single-file download creates useful SFTP download log metadata.
- [ ] Single-file upload creates useful SFTP upload log metadata.
- [ ] Queue with multiple downloads logs queue started, item started/completed, and queue finished.
- [ ] Queue with multiple uploads logs queue started, item started/completed, and queue finished.
- [ ] Folder upload/download logs folder confirmation/selection summary and queue item outcomes.
- [ ] Cancelled transfer logs a WARN line and does not log file contents.
- [ ] Skipped overwrite / Skip all / Overwrite all decisions are logged.
- [ ] Retry selected logs requeued counts.
- [ ] Disable diagnostic logging and confirm no new transfer events are appended afterward.

## Negative log-content checks

Search the log and confirm it does **not** contain:

- [ ] passwords
- [ ] private key contents
- [ ] plaintext secret values from `dd-ssh.json`
- [ ] terminal commands or terminal output
- [ ] clipboard contents
- [ ] file contents
- [ ] full `dd-ssh.json` content

## Regression checks

- [ ] Terminal still opens.
- [ ] Paste / Ctrl+C still work.
- [ ] File Manager still opens.
- [ ] Queue upload/download still work.
- [ ] Folder upload/download still work.
- [ ] Retry selected still works.
- [ ] Overwrite all / Skip all still work.
