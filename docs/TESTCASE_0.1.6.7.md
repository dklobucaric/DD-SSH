# Testcase — dev 0.1.6.7

**Checkpoint:** dev 0.1.6.7 — Andromeda
**Feature:** Basic Session Traffic Monitor

## Goal

Validate that DD-SSH displays live in/out traffic for the active terminal tab and logs only useful traffic lifecycle summaries when diagnostic logging is enabled.

## Preconditions

- At least two saved SSH sessions are useful, but one is enough for the core test.
- Diagnostic logging should be tested both OFF and ON.
- Existing SSH trust-chain, known-host multi-key behavior, and terminal operation must remain unchanged.

## Test 1 — initial state

Expected on startup:

```text
Traffic: No active session
```

## Test 2 — successful terminal traffic

1. Open a saved xterm.js terminal.
2. Run:

```bash
whoami
ls -la
```

Expected:

```text
Traffic: <session> ↓ <rate> ↑ <rate> | Total ↓ <value> ↑ <value>
```

The received total should increase when server output arrives. The sent total should increase when keyboard input or paste is sent.

## Test 3 — active tab follows selection

1. Open two saved terminal tabs.
2. Run output-heavy commands in each.
3. Switch tabs.

Expected:

```text
Traffic: doma ...
Traffic: printer ...
```

The status bar should follow the active tab, not all sessions combined.

## Test 4 — disconnect behavior

1. Open a terminal.
2. Generate some traffic.
3. Disconnect the session.

Expected:

```text
Traffic: <session> disconnected ↓ 0 B/s ↑ 0 B/s | Total ↓ <value> ↑ <value>
```

## Test 5 — logging OFF

1. Disable diagnostic logging.
2. Open a terminal and generate traffic.

Expected:

- Live traffic widget works.
- No log file is created or appended because logging is disabled.

## Test 6 — logging ON

1. Enable diagnostic logging.
2. Open a terminal.
3. Generate traffic.
4. Disconnect.
5. Open Help → Open Log Folder.

Expected log contains lifecycle/summary entries similar to:

```text
Traffic monitor started: session="doma"
Session traffic summary: session="doma", duration=00:04:12, received=3.8 MB, sent=220 KB
Traffic monitor stopped: session="doma", received=3.8 MB, sent=220 KB
```

Expected log does not contain:

```text
password values
private key contents
terminal input
terminal output
clipboard contents
full dd-ssh.json
```

## Test 7 — regression checks

Confirm still working:

```text
saved sessions load
xterm.js terminal opens
wrong password produces WARN in log when logging is enabled
known-host verification before auth still logs OK/failure
Windows KEX workaround still works on the known regression host
macOS DMG still launches on a clean Mac
```
