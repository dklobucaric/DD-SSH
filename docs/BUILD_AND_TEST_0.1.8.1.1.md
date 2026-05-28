# Build and Test — dev 0.1.8.1.1

Checkpoint: `dev 0.1.8.1.1` — Queue selected UI consolidation.

This checkpoint adds experimental recursive folder upload/download by scanning folders and expanding them into the existing sequential transfer queue. It does not add sync, parallel transfer, resume, permission/timestamp preservation, delete/rename/chmod, or SFTP traffic monitor integration.

## Build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Smoke test

- About dialog shows `dev 0.1.8.1.1`.
- Existing terminal sessions still open and paste/Ctrl+C still work.
- File Manager opens.
- Queue upload/download buttons are visible.
- A small local folder uploads recursively to the current remote directory.
- A small remote folder downloads recursively to the current local directory.
- Queue execution remains one item at a time.
- Retry selected, Overwrite all, Skip all, Cancel item, and Clear finished still work.

## First recommended test folder

Use a tiny folder first:

```text
test-folder/
  a.txt
  b.txt
  sub/
    c.txt
```

Do not start by selecting `/`, `/home`, `C:\`, or a production backup directory. This is intentionally still an experiment checkpoint.


## Extra checks for dev 0.1.8.1.1

- Confirm the local panel has one queue button: `Queue upload`.
- Confirm the remote panel has one queue button: `Queue download`.
- Confirm separate `Queue folder upload` / `Queue folder download` buttons are no longer visible.
- Select one local file and one local folder, then click `Queue upload`; the file should queue directly and the folder should ask for recursive confirmation.
- Select one remote file and one remote folder, then click `Queue download`; the file should queue directly and the folder should ask for recursive confirmation.
