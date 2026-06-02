# Build and test — dev 0.1.8.8

Checkpoint: **dev 0.1.8.8 — Settings dialog layout polish**

This checkpoint changes the Settings dialog layout only. It should not change SSH, terminal, SFTP/File Manager, queue/delete, logging runtime, config schema, or packaging behavior.

## Linux quick build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## macOS quick build

```bash
cd ~/DD-SSH
rm -rf build-macos-release
./scripts/macos-build-release.sh
open "$HOME/DD-SSH/build-macos-release/dd-ssh.app"
```

## Focus

Open **Tools → Settings** and verify the new category-sidebar layout, readable config path, Copy path/Open folder actions, and save/cancel behavior.
