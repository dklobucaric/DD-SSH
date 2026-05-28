# Build and test — dev 0.1.8.0.2

Checkpoint: **dev 0.1.8.0.2 — Transfer queue overwrite-all polish**

This is a small queue UX polish checkpoint on top of `dev 0.1.8.0.1`. It adds `Overwrite all` and `Skip all` choices so repeated queue overwrite conflicts do not require one dialog per file.

## Build

```bash
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Scope

Included:

- Queue overwrite dialog now offers `Overwrite`, `Skip`, `Overwrite all`, `Skip all`, and `Cancel queue`.
- `Overwrite all` applies to remaining queued conflicts of the same direction during the current queue run.
- `Skip all` applies to remaining queued conflicts of the same direction during the current queue run.
- Download overwrite prompts still appear before progress starts.
- Upload overwrite prompts still appear visibly after any temporary progress dialog closes.

Not included:

- Folder transfer.
- Parallel transfer.
- Retry failed items.
- Sync engine.
- SFTP traffic monitor integration.
- Terminal/SSH core changes.

## Smoke test

1. Open DD-SSH.
2. Confirm Help → About shows `dev 0.1.8.0.2`.
3. Open File Manager for a saved session.
4. Queue several downloads where multiple target files already exist locally.
5. Start queue and choose `Overwrite all` on the first conflict.
6. Expected: remaining download overwrite conflicts continue without repeated prompts.
7. Repeat with `Skip all`.
8. Expected: remaining existing local target files are skipped.
9. Queue several uploads where multiple target files already exist remotely.
10. Repeat `Overwrite all` and `Skip all` tests for upload conflicts.
