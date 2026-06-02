# Build and test — dev 0.1.8.9

Checkpoint: `dev 0.1.8.9 — SFTP traffic monitor integration`

This checkpoint extends the existing status-bar Session Traffic widget so active File Manager SFTP upload/download bytes are visible while transfers run. It does not change SSH auth, known-host behavior, terminal paste, SFTP transfer semantics, queue/delete behavior, logging privacy rules, config schema, or packaging scripts.

## Linux build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## macOS release smoke

```bash
cd ~/DD-SSH
rm -rf build-macos-release
./scripts/macos-build-release.sh
open "/Users/user/DD-SSH/build-macos-release/dd-ssh.app"
```

## Windows release smoke

Use the existing MSVC/Qt/vcpkg release build flow, then open the app and confirm `Help → About` shows `dev 0.1.8.9`.

## Expected behavior

- Terminal tabs still show SSH shell-channel traffic.
- File Manager tabs now show SFTP upload/download traffic when selected.
- Download bytes increase the down/received total.
- Upload bytes increase the up/sent total.
- Queue-expanded folder transfers update the same counters because they run as normal queue upload/download items.
- Delete/mkdir/listing operations do not show as bulk transfer traffic.
