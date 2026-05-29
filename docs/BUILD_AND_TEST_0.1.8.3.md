# Build and Test — dev 0.1.8.3

Checkpoint: `dev 0.1.8.3` — File transfer logging and diagnostics.

This checkpoint adds diagnostic logging around SFTP/File Manager workflows while preserving the existing transfer runtime.

## Build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Smoke test

- About shows `dev 0.1.8.3`.
- Diagnostic logging is OFF by default.
- Enable diagnostic logging in Settings.
- Use Help → Open Log Folder to locate today's `yyyymmdd.log`.
- Run single-file upload/download.
- Run queue upload/download with multiple files.
- Run folder upload/download with a small test folder.
- Test cancel and skipped/overwrite scenarios.
- Confirm log entries are useful and contain no secrets or file contents.

## Expected log safety

Logs may contain metadata such as paths, byte counts, elapsed times, queue counts, statuses, and error messages. Logs must not contain passwords, private keys, plaintext secret values, terminal input/output, clipboard contents, or file contents.
