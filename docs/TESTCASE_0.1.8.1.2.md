# Testcase — dev 0.1.8.1.2

Checkpoint: `dev 0.1.8.1.2` — Tester release polish.

## Version and docs

- [ ] About dialog shows `dev 0.1.8.1.2`.
- [ ] Welcome tab mentions the current tester-ready file-transfer baseline.
- [ ] `docs/TESTER_CHECKLIST_0.1.8.1.2.md` exists.
- [ ] `docs/KNOWN_LIMITATIONS.md` clearly mentions plaintext secrets and experimental folder transfer.
- [ ] `docs/TEST_MATRIX.md` includes a `dev 0.1.8.1.2` section.

## Regression smoke test

- [ ] Existing SSH terminal opens from a saved session.
- [ ] Paste and Ctrl+C still work.
- [ ] File Manager opens from a saved session.
- [ ] Local panel shows `Queue upload`.
- [ ] Remote panel shows `Queue download`.
- [ ] Single-file download still works.
- [ ] Single-file upload still works.
- [ ] Queue multiple files and run them sequentially.
- [ ] Retry selected moves Done/Failed/Cancelled/Skipped items back to Pending.
- [ ] Overwrite all and Skip all still work.
- [ ] Small local folder upload works.
- [ ] Small remote folder download works.
- [ ] Exit safety still warns about open terminal/file manager/queue work.

## Third-party tester minimum report format

Ask each tester to report:

```text
OS/version:
DD-SSH version:
Build type/package:
Server OS/OpenSSH version if known:
Auth type: password/private key
Feature tested:
Expected result:
Actual result:
Screenshots/log excerpt:
Can reproduce: yes/no
```
