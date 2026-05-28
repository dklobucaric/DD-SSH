# Testcase — dev 0.1.7.2

**Checkpoint:** dev 0.1.7.2 — Andromeda  
**Feature:** File Transport architecture/design foundation  
**Expected risk:** low, because SSH terminal runtime is intentionally not changed

---

## Preconditions

Use a working `dev 0.1.7.1` config with at least one saved session.

Recommended regression sessions:

```text
[ ] one password-auth session
[ ] one private-key-auth session if available
[ ] one known-host multi-key portability host if available
[ ] Windows KEX regression host if testing on Windows
```

Back up real `dd-ssh.json` before testing if needed.

---

## Test 1 — version and startup

Steps:

```text
1. Start DD-SSH.
2. Open Help → About DD-SSH.
```

Expected:

```text
[ ] App starts without crash
[ ] About shows Version: dev 0.1.7.2
[ ] Codename remains Andromeda
[ ] Current phase mentions File Transfer architecture foundation
```

---

## Test 2 — sidebar session context menu

Steps:

```text
1. Right-click a saved session in the sidebar.
2. Inspect the context menu.
3. Click Open File Manager (planned).
```

Expected:

```text
[ ] Menu contains Open xterm.js terminal
[ ] Menu contains Open File Manager (planned)
[ ] Clicking Open File Manager (planned) opens an information dialog
[ ] Dialog says no SFTP connection is opened yet
[ ] Dialog references future 0.1.7.x SFTP development
[ ] No terminal tab is opened by this action
[ ] No file list is displayed
[ ] No config is changed
```

---

## Test 3 — config remains unchanged by placeholder

Steps:

```bash
cp ~/.config/DD-Lab/DD-SSH/dd-ssh.json /tmp/dd-ssh-before.json
# Use Open File Manager (planned) on one or more sessions.
cmp ~/.config/DD-Lab/DD-SSH/dd-ssh.json /tmp/dd-ssh-before.json
```

Expected:

```text
[ ] cmp returns no differences
```

If diagnostic logging is enabled, a log line for the placeholder may be written to the log file. The config file itself should not change.

---

## Test 4 — xterm.js terminal still works

Steps:

```text
1. Double-click a saved session.
2. Connect normally.
3. Run basic commands.
```

Commands:

```bash
whoami
pwd
clear
stty size
printf 'čćžšđ ČĆŽŠĐ € 🚀\n'
```

Expected:

```text
[ ] Terminal opens normally
[ ] Commands execute normally
[ ] UTF-8 text renders correctly
[ ] PTY size is reported normally
[ ] Session Traffic shows activity
```

---

## Test 5 — paste regression

Steps:

```text
1. Copy a short command to clipboard.
2. Paste using toolbar Paste.
3. Paste using right-click paste.
4. Paste using Ctrl+Shift+V on Linux/Windows or Command+V on macOS.
```

Expected:

```text
[ ] Pasted text appears once
[ ] No raw ^[[200~ or ^[[201~ markers appear
[ ] Large paste behavior from dev 0.1.7.1 is not worse
```

---

## Test 6 — Ctrl+C interrupt regression

Steps:

```bash
seq 1 500000
```

While output is running, press keyboard Ctrl+C inside the focused terminal.

Expected:

```text
[ ] Remote command is interrupted
[ ] Terminal remains usable
[ ] Copy toolbar button behavior is unchanged
```

---

## Test 7 — diagnostic logging privacy

Steps:

```text
1. Enable diagnostic logging in Settings.
2. Open File Manager (planned) from a saved-session context menu.
3. Open a terminal and run a few commands.
4. Open Help → Open Log Folder.
5. Inspect today's log.
```

Expected:

```text
[ ] Log may mention File Manager placeholder opened
[ ] Log does not contain password values
[ ] Log does not contain private key contents
[ ] Log does not contain terminal input/output contents
[ ] Log does not contain clipboard contents
[ ] Log does not contain full dd-ssh.json
```

---

## Test 8 — known-host / Windows KEX regression smoke

Use the same hosts that passed in `dev 0.1.7.1`.

Expected:

```text
[ ] Existing trusted hosts still connect
[ ] Known-host multi-key host still works
[ ] Windows OpenSSH 10 / KEX regression host still connects on Windows builds
```

---

## Pass criteria

`dev 0.1.7.2` passes when:

```text
[ ] Version/docs are updated
[ ] File Manager placeholder is visible and harmless
[ ] No SFTP runtime is accidentally introduced
[ ] Terminal baseline from dev 0.1.7.1 remains stable
[ ] Config remains human-readable and unchanged by placeholder
[ ] Logging remains opt-in and secret-safe
```
