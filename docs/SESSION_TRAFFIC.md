# DD-SSH Session Traffic Monitor

**Checkpoint:** dev 0.1.7.0 — Andromeda

DD-SSH includes a compact live traffic indicator in the status bar for the active terminal tab.

Example:

```text
Traffic: doma ↓ 4.2 KB/s ↑ 0.4 KB/s | Total ↓ 1.1 MB ↑ 80 KB
```

## What it measures

For this checkpoint, the monitor measures application-level SSH shell-channel traffic:

```text
received = bytes read by DD-SSH from the SSH shell channel
sent     = bytes successfully written by DD-SSH to the SSH shell channel
```

It follows the active terminal tab. If you switch from one terminal tab to another, the status bar switches to the selected session.

## What it does not measure

The monitor does not measure:

```text
global OS network traffic
traffic from other applications
TCP/VPN/protocol overhead
SFTP/file-transfer traffic yet
```

This is intentional. The current feature is a small terminal-channel monitor and a foundation for future file transport visibility.

## Logging behavior

When diagnostic logging is enabled, DD-SSH logs traffic lifecycle and summary events only:

```text
Traffic monitor started: session="doma"
Session traffic summary: session="doma", duration=00:04:12, received=3.8 MB, sent=220 KB
Traffic monitor stopped: session="doma", received=3.8 MB, sent=220 KB
```

The live UI updates periodically, but the log does not record every refresh or every byte.

## Privacy

Traffic logs do not contain terminal input, terminal output, passwords, private-key contents, clipboard contents, or full JSON config data.

## Future direction

The feature is named Session Traffic rather than Terminal Traffic so future DD-SSH transport features can extend the same UI and diagnostic model:

```text
terminal channel traffic
future SFTP/file-transfer traffic
future per-session totals
```
