# Testcase — dev 0.1.6.6

**Checkpoint:** dev 0.1.6.6 — Andromeda  
**Feature:** optional diagnostic logging foundation

## Test 1 — default OFF

1. Start DD-SSH with an existing config or fresh config.
2. Open Tools → Settings.
3. Verify Enable diagnostic logging is unchecked by default.
4. Open Help → Open Log Folder.

Expected:

```text
Log folder opens.
No new log file is required unless logging was enabled.
```

## Test 2 — enable logging without restart

1. Open Tools → Settings.
2. Check Enable diagnostic logging.
3. Save settings.
4. Verify the status bar shows Logging enabled.
5. Open Help → Open Log Folder.
6. Open today's `yyyymmdd.log` file.

Expected log examples:

```text
2026-05-28 01:44:22.123 - INFO - Diagnostic logging enabled
2026-05-28 01:44:22.124 - INFO - Settings saved
```

No app restart should be required.

## Test 3 — append same-day log

1. Keep logging enabled.
2. Close DD-SSH.
3. Reopen DD-SSH.
4. Open today's log.

Expected:

```text
The same yyyymmdd.log file is appended.
A new App started line appears.
```

## Test 4 — normal SSH success path

1. Keep logging enabled.
2. Open a known/trusted saved session.
3. Run a simple command such as `whoami`.
4. Disconnect or close the tab.

Expected log includes lifecycle events such as:

```text
SSH shell worker started
SSH shell connect started
SSH shell connect OK
SSH shell host-key verification OK
SSH shell authentication started
SSH shell authentication successful
SSH shell channel open
SSH shell disconnected
```

Expected log does **not** include terminal input/output such as `whoami` or command results.

## Test 5 — wrong password / auth failure

1. Enable logging.
2. Run a manual connection test or saved-session test with a wrong password.

Expected:

```text
Authentication failure is logged as WARN.
No password value is logged.
```

## Test 6 — host-key / trust-chain regression

1. Enable logging.
2. Use an existing trusted host.
3. Open terminal.

Expected:

```text
Host-key verification before auth is logged as OK.
Authentication happens only after that check.
```

## Test 7 — disable logging without restart

1. Open Tools → Settings.
2. Uncheck Enable diagnostic logging.
3. Save settings.
4. Verify status-bar indicator disappears.
5. Perform another simple UI action.

Expected:

```text
A Diagnostic logging disabled line is written.
After that, no new lines are appended while logging is OFF.
```

## Secret-safety checklist

Search the log for sensitive text before sending it:

```text
[ ] no password values
[ ] no private key contents
[ ] no terminal command input
[ ] no terminal command output
[ ] no full dd-ssh.json contents
```
