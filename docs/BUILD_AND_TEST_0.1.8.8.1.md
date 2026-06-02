# Build and test — dev 0.1.8.8.1

Checkpoint: **dev 0.1.8.8.1 — Settings dialog General page polish**

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


## Additional 0.1.8.8.1 check

- [ ] On macOS, the General page shows the config path field on its own row.
- [ ] Copy path, Open folder, and the info hint are visible below the config path field.
- [ ] The “Read-only path…” description appears below the buttons.
