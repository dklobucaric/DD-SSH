# Build and test — dev 0.1.8.0.1

Checkpoint: **dev 0.1.8.0.1 — Transfer queue overwrite prompt polish**

This is a small bugfix checkpoint on top of `dev 0.1.8.0`. It fixes a blocking queue UX bug where an overwrite prompt could appear underneath the queue progress dialog, making the transfer look stuck at 0%.

## Build

```bash
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Scope

Included:

- Queue download overwrite prompt appears before the queue progress dialog for that item.
- Queue upload overwrite prompt closes the temporary progress dialog before asking the user.
- Queue cancel/continue prompts appear after progress is closed.
- Skipped/cancelled queue item states are preserved.

Not included:

- Folder transfer.
- Parallel transfer.
- Retry failed items.
- Sync engine.
- SFTP traffic monitor integration.
- Terminal/SSH core changes.

## Smoke test

1. Open DD-SSH.
2. Confirm Help → About shows `dev 0.1.8.0.1`.
3. Open File Manager for a saved session.
4. Queue two or more uploads where at least one target already exists remotely.
5. Start queue.
6. Confirm the overwrite prompt is visible on top and not hidden behind progress.
7. Choose No and confirm the item becomes Skipped and the queue continues.
8. Repeat with Yes and confirm upload continues.
9. Repeat with Cancel and confirm queue stops cleanly.
10. Repeat equivalent download overwrite test with an existing local target file.
