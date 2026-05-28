# Build and Test — dev 0.1.8.1.2

Checkpoint: `dev 0.1.8.1.2` — Tester release polish.

This checkpoint prepares the accepted `dev 0.1.8.1.1` File Manager baseline for third-party alpha testing. It updates documentation and in-app status text only. It does not intentionally change transfer runtime behavior.

## Build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Smoke test

- About dialog shows `dev 0.1.8.1.2`.
- Welcome tab describes the tester-ready file-transfer baseline.
- Existing SSH terminal sessions still open.
- Paste and Ctrl+C still behave as before.
- File Manager opens from a saved session.
- Local panel shows `Queue upload`.
- Remote panel shows `Queue download`.
- Single-file upload/download still work.
- Small-folder upload/download still work through queue expansion.
- Retry selected still works.
- Overwrite all / Skip all still work.
- Exit safety still reports open terminal/file manager/queue work.

## Tester handoff

Give testers:

- the app build for their platform
- `README.md`
- `docs/TESTER_CHECKLIST_0.1.8.1.2.md`
- `docs/KNOWN_LIMITATIONS.md`
- `docs/TESTCASE_0.1.8.1.2.md`

Ask testers to start with test servers and small test folders. Do not start with production directories, `/`, `/home`, `C:\`, or huge backup trees.

## Expected result

If this checkpoint behaves correctly, runtime behavior should match `dev 0.1.8.1.1`; the visible difference is clearer documentation/status wording for alpha testers.
