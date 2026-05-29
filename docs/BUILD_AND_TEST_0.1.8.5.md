# Build and Test — dev 0.1.8.5

Checkpoint: `dev 0.1.8.5` — Remote queue delete experiment.

## Build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Smoke test

- About shows `dev 0.1.8.5`.
- Existing terminal behavior still works.
- File Manager opens.
- Upload/download/queue/folder queue behavior from `0.1.8.4` still works.
- Remote panel shows `Queue delete`.
- Queue delete can add a remote regular file to the queue after confirmation.
- Starting a queue with pending delete item(s) shows a destructive confirmation.
- Confirmed delete removes the remote file and refreshes the remote panel.
- Cancelling the delete-run confirmation leaves delete item(s) Pending.
- Empty remote directory delete may succeed; non-empty remote directory delete should fail safely.
- Diagnostic logging, when enabled, records delete decisions/outcomes but no secrets or file contents.
