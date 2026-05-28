# Build and test — dev 0.1.7.4.1

Checkpoint: `dev 0.1.7.4.1 — Read-only SFTP browser bugfix polish`

This checkpoint polishes the first read-only SFTP browser before committing the 0.1.7.4 line. It should be tested on Linux first, then smoke-tested on Windows and macOS.

## Build on Linux

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## What changed

- App close confirmation now includes open SFTP browser tabs as well as active SSH terminal sessions.
- SFTP browser table alternating row colors are disabled for readable dark-theme rows.
- Tab bar scroll-button hints are enabled for crowded tab bars.
- `Up` button label is `↑ Up`.
- Documentation notes that SFTP traffic is not yet part of the live Session Traffic monitor.

## What did not change

- No upload/download.
- No local file browser panel.
- No SFTP traffic monitor integration yet.
- No path normalization polish for `.` / `..` beyond the existing 0.1.7.4 behavior.
- No SSH core, terminal transport, known-host, Windows KEX, config schema, or packaging behavior was intentionally changed.

## Cross-platform smoke

- Linux: full smoke test.
- Windows: build + open browser + table visibility + exit safety.
- macOS: build + crowded tab bar behavior + open browser + table visibility + exit safety.
