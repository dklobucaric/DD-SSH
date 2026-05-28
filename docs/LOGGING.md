# DD-SSH diagnostic logging

**Checkpoint:** dev 0.1.6.7 — Andromeda  
**Scope:** optional diagnostic logging foundation plus Session Traffic summaries

DD-SSH diagnostic logging is an on-demand debugging tool for testers and developers. It is **off by default** and can be enabled only when needed.

## Enable / disable logging

Open:

```text
Tools → Settings → Enable diagnostic logging
```

When enabled, DD-SSH writes diagnostic events to a standard per-user log folder. The setting applies immediately after saving Settings; no app restart should be required.

When logging is enabled, the status bar shows:

```text
Logging enabled
```

## Open the log folder

Open:

```text
Help → Open Log Folder
```

This opens the platform-specific folder in the normal file manager.

## Log file name

DD-SSH creates one log file per local day:

```text
20260528.log
```

If the file already exists, DD-SSH appends to it. If diagnostic logging is disabled, DD-SSH does not create log files.

## Log line format

```text
2026-05-28 01:44:22.123 - INFO - App started
```

Visible log levels in this checkpoint:

```text
INFO
WARN
ERROR
```

## Log locations

The log folder is user-writable and does not require administrator/root privileges.

| Platform | Log folder |
|---|---|
| Linux | `~/.local/state/DD-SSH/logs` |
| Windows | `%LOCALAPPDATA%\DD-SSH\logs` |
| macOS | `~/Library/Logs/DD-SSH` |

## What is logged

When diagnostic logging is enabled, DD-SSH logs useful lifecycle and troubleshooting events, such as:

```text
App started
App version
Config file path
Log folder path
Saved session list loaded
Manual/saved connection requested
SSH handshake started / OK / failed
Host-key verification before auth
Authentication started / success / failed
Private-key load errors
Shell channel open / disconnect
Session traffic monitor start/stop
Session traffic summary on disconnect
App closing
```

## What is never logged

DD-SSH diagnostic logs must not contain secrets or terminal content.

Never log:

```text
password values
private key contents
plain secret values
terminal input
terminal output
clipboard contents
full dd-ssh.json contents
```

It is OK to log metadata such as auth method (`password` or `private-key`), host, port, username, key type, and host-key fingerprint because these are needed for troubleshooting SSH trust/auth issues.

## Log rotation

No automatic rotation/pruning is implemented in this checkpoint.

Reason: diagnostic logging is off by default and intended for short, on-demand troubleshooting sessions. If logs become too large in future public testing, a later version can add a simple policy such as keeping the last 10 log files or the last 14 days.

## Future extensions

The logger is intentionally generic so future features can use it without redesign. The `dev 0.1.6.7` Session Traffic monitor uses the logger for lifecycle and summary events:

```text
Traffic monitor started: session="doma"
Session traffic summary: session="doma", duration=00:04:12, received=3.8 MB, sent=220 KB
Traffic monitor stopped: session="doma", received=3.8 MB, sent=220 KB
```

The live traffic monitor itself should update in the UI, not spam the log every millisecond.
