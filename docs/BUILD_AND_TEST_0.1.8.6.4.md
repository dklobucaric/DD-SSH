# Build and test — dev 0.1.8.6.4

`dev 0.1.8.6.4` fixes queue delete confirmation ordering. Destructive delete confirmations are now shown when the queue reaches the delete item, not before the queue run starts.

## Linux build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Smoke test

- About shows `dev 0.1.8.6.4`.
- Queue an upload that conflicts with an existing remote file, then a local delete item.
- Start queue: the upload overwrite dialog should appear before the local delete confirmation.
- Queue a download that conflicts with an existing local file, then a remote delete item.
- Start queue: the download overwrite dialog should appear before the remote delete confirmation.
- If delete is not confirmed, the delete item remains Pending and queue stops before it.
- Existing upload/download/delete queue behavior still works.
- Terminal paste formatting from `dev 0.1.8.6.2` still works.
