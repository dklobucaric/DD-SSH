# Testcase — dev 0.1.6.8

**Checkpoint:** dev 0.1.6.8 — Andromeda  
**Feature:** Config import/export safety preview.

## Expected unchanged behavior

- SSH connections still work.
- Session Traffic still works.
- Diagnostic logging still works.
- `dd-ssh.json` remains human-readable JSON.
- No config schema migration is required.

## Test 1 — export preview

1. Start DD-SSH.
2. Use File → Export Config.
3. Confirm the preview dialog appears before file save/export.
4. Confirm it shows:
   - session count
   - known-host count
   - trusted key count
   - secrets mode
   - saved secret count
   - plaintext-secret warning if applicable
5. Continue and export to a test JSON file.
6. Confirm export succeeds.

## Test 2 — import preview with exported config

1. Use File → Import Config.
2. Select the exported JSON file.
3. Confirm the import preview appears before replacement.
4. Cancel.
5. Confirm active sessions/settings were not replaced.

## Test 3 — invalid JSON import

Create a broken file:

```bash
echo '{ broken json' > /tmp/dd-ssh-broken.json
```

Import it. Expected:

- DD-SSH refuses the import.
- Error mentions invalid JSON and offset.
- Active config is not replaced.

## Test 4 — root array import

Create a JSON array:

```bash
echo '[]' > /tmp/dd-ssh-array.json
```

Import it. Expected:

- DD-SSH refuses the import.
- Error says root must be a JSON object.
- Active config is not replaced.

## Test 5 — diagnostic logging

With diagnostic logging enabled, repeat export/import preview. Expected log examples:

```text
Config export preview: sessions=..., known_hosts=..., secrets_mode=plain-v1, plaintext_secrets=YES
Config import preview: sessions=..., known_hosts=..., secrets_mode=plain-v1, plaintext_secrets=YES
```

The log must not contain password values, private-key contents, full config JSON, terminal input/output, or clipboard content.
