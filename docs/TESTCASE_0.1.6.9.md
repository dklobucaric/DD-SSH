# Testcase — dev 0.1.6.9

**Checkpoint:** dev 0.1.6.9 — Andromeda
**Type:** Bugfix stabilization.

## Expected unchanged behavior

- SSH connections still work.
- Host-key verification before auth still works.
- Diagnostic logging still works.
- Session Traffic still works.
- Config import/export preview still works.
- `dd-ssh.json` remains human-readable JSON.

## Test 1 — xterm.js toolbar label and copy behavior

1. Open a saved session with the xterm.js terminal.
2. Confirm the toolbar shows:
   - `Copy`
   - `Paste`
   - `Clear`
   - `Reset`
   - `Focus`
   - `Reconnect`
   - `Disconnect`
3. Select visible text inside the terminal.
4. Click `Copy`.
5. Paste into a local text editor.
6. Expected: selected terminal text is copied.
7. Expected: keyboard Ctrl+C inside the terminal can still interrupt remote programs.

## Test 2 — BasicTerminal fallback label

If the fallback/basic terminal is used, confirm the remote interrupt button says `Interrupt`, not `Ctrl+C`.

## Test 3 — import config dialog action label

1. Use File → Import Config.
2. Select a valid DD-SSH JSON config.
3. Confirm the preview dialog has `Import` and `Cancel` buttons.
4. Click `Cancel`. Expected: active config is unchanged.
5. Repeat and click `Import`. Expected: config is replaced and sessions/settings reload without app restart.

## Test 4 — restore latest backup dialog action label

1. Ensure at least one `dd-ssh.json.bak-*` backup exists.
2. Use File → Restore Latest Backup.
3. Confirm the confirmation dialog has `Restore` and `Cancel` buttons.
4. Click `Cancel`. Expected: active config is unchanged.
5. Repeat and click `Restore`. Expected: newest valid backup is restored and the previous active config is moved aside as `dd-ssh.json.pre-restore-*`.

## Test 5 — regression smoke

1. Open a trusted saved session.
2. Confirm terminal connects.
3. Confirm Session Traffic updates.
4. Enable diagnostic logging.
5. Reconnect and disconnect.
6. Confirm logs remain readable and contain no password/private-key/terminal data.
