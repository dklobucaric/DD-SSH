# DD-SSH dev 0.1.7.1 — Test case

## Scope

Focused bugfix for native/browser paste paths in the xterm.js terminal.

## Test data

Copy this command to the local clipboard:

```bash
printf 'UTF-8 test: čćžšđ ČĆŽŠĐ € 🚀\n'
```

## Tests

1. Start DD-SSH and confirm About shows `dev 0.1.7.1`.
2. Open a saved xterm.js terminal session.
3. Paste using the toolbar Paste button. Expected: command executes cleanly and prints UTF-8 text.
4. Paste using right-click paste. Expected: command executes cleanly; no literal `^[[200~` or `^[[201~` appears.
5. Paste using Ctrl+Shift+V on Linux/Windows. Expected: command executes cleanly; no literal `^[[200~` or `^[[201~` appears.
6. Paste using Command+V on macOS. Expected: command executes cleanly; no literal `^[[200~` or `^[[201~` appears.
7. Run `seq 1 500000`, then press keyboard Ctrl+C inside the terminal. Expected: remote command stops and prompt returns.
8. Paste 200–900 KB of safe test text through the toolbar Paste button into a test file. Expected: file size is not truncated.
9. Confirm Session Traffic counters still update during output-heavy commands.
10. If diagnostic logging is enabled, confirm logs do not contain terminal input/output or clipboard content.

## Non-goals

This checkpoint does not change SSH trust-chain logic, JSON config format, logging settings, Session Traffic design, packaging, or file transfer behavior.
