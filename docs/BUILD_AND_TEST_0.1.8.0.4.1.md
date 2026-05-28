# Build and Test — dev 0.1.8.0.4.1

Checkpoint: `dev 0.1.8.0.4.1` — Transfer queue stabilization polish.

## Scope

This checkpoint stabilizes the existing transfer queue before folder-transfer work. It does not add folder transfer or change the SSH/terminal core.

## Build

```bash
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Main validation

- About dialog shows `dev 0.1.8.0.4.1`.
- File Manager opens from a saved session.
- Queue download/upload still works for multiple individual files.
- `Retry selected` still works for `Done`, `Failed`, `Cancelled`, and `Skipped` items.
- `Overwrite all` / `Skip all` behavior still works.
- Closing DD-SSH with running or pending queue work shows that queue state in the exit safety dialog.
- Queue/navigation controls are disabled while a queue run is active.

## Expected non-features

- Folder transfer is not implemented yet.
- Parallel transfers are not implemented.
- Resume and sync are not implemented.
- SFTP traffic is not included in the Session Traffic monitor yet.


## Compile hotfix note

This hotfix fixes a broken multi-line QStringLiteral in `src/ui/SftpBrowserTab.cpp` that prevented `dev 0.1.8.0.4` from compiling. The queue stabilization behavior remains the same.
