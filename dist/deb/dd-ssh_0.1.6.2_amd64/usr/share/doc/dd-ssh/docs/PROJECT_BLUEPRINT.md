# DD-SSH Project Blueprint

## Project summary

DD-SSH is a clean cross-platform SSH client and session manager.

Primary stack:

```text
C++
Qt 6
CMake
libssh
xterm.js through Qt WebEngine
JSON config
```

Primary platforms:

```text
Linux
Windows
macOS
```

Current tested focus: Linux.

## Product philosophy

```text
Fast to open.
Fast to connect.
Easy to understand.
Easy to sync.
Easy to back up.
Hard to accidentally destroy things.
No unnecessary circus.
```

## What DD-SSH is

- Desktop SSH client
- Saved session manager
- xterm.js terminal frontend
- Single JSON config app
- Practical sysadmin tool
- Open-source project

## What DD-SSH is not yet

- Stable 1.0 product
- SFTP client
- Split-pane terminal suite
- Cloud sync service
- Team collaboration system
- Encrypted secret vault
- Multi-exec power tool

## Current Andromeda direction

The 0.1.x Andromeda line is building toward:

```text
MF 0.2 — Real Terminal Foundation
```

That means:

- Saved sessions work
- SSH auth works
- known_hosts works
- xterm terminal works
- PTY resize works
- terminal lifecycle works
- config safety exists
- public alpha docs exist

## Core data model

`dd-ssh.json` is the source of truth for:

```text
settings
sessions
known_hosts
secrets
metadata
```

Secrets are currently plaintext under `secrets.mode = plain-v1`.

## Architectural rules

- Do not put SSH protocol logic into UI classes when a core/ssh class can own it.
- Do not block the GUI thread with SSH read loops.
- Do not silently accept changed host keys.
- Do not write secrets to logs.
- Do not overwrite corrupt configs automatically.
- Do not change config format without updating docs.
- Keep terminal frontend replaceable enough for future changes.

## Near-term goals

- Finish public alpha documentation
- Stabilize config workflow
- Prepare v0.2.0-alpha Andromeda

## Later goals

- Custom config path / portable mode
- Keep-alive
- Multi-Exec
- Cross-platform packaging
- Encrypted secrets
- Public releases
