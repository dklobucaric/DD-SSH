# Build and test — dev 0.1.8.6.3

## Scope

`dev 0.1.8.6.3` is a UI wording and File Manager polish checkpoint. It keeps the accepted `dev 0.1.8.6.2` terminal paste newline fix and does not change SSH/SFTP transfer core behavior.

## Build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Expected checks

- About shows `dev 0.1.8.6.3`.
- Saved-session context menu labels are concise:
  - `Open terminal`
  - `Open file manager`
  - `Run auth test`
  - `Open fallback shell`
  - `Edit session`
  - `Delete session`
- About dialog shows Developer in the upper metadata area, then Version, Codename, Current phase, and Milestone.
- About dialog includes `Developer: DD-LAB vl. Dalibor Klobučarić`.
- Local File Manager toolbar shows `Queue upload` before `Delete local`.
- Remote File Manager toolbar shows `Queue download` and `Delete remote`; the legacy `Download selected now` toolbar button is not visible.
- Queue upload/download/delete, folder queue, Retry selected, Overwrite all / Skip all, diagnostic logging, and terminal paste still work.
