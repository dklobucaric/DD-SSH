# DD-SSH Project Blueprint v0.1

**Project name:** DD-SSH  
**Project type:** Cross-platform SSH client / session manager  
**Primary platforms:** Linux, Windows, macOS  
**Project status:** Planning / architecture phase  
**Primary goal:** Build a clean, lightweight, practical SSH client inspired by the simplicity of PortX, but focused only on the features that matter for everyday sysadmin work.

---

## 1. Executive Summary

DD-SSH is intended to be a cross-platform SSH terminal client and session manager built for users who manage multiple servers and want a simple, fast, reliable tool without enterprise bloat, cloud lock-in, SFTP panels, split screens, telemetry, or unnecessary visual noise.

The application should run on Linux, Windows, and macOS from a shared codebase. It should eventually be distributed through multiple channels: source code on GitHub, Linux packages, Windows installer, macOS app bundle/DMG, and potentially official stores where feasible.

The project should be built carefully and incrementally. The first major technical milestone is not packaging or store upload. The first milestone is a stable SSH terminal proof-of-concept.

The core idea:

```text
One clean SSH app.
One consistent config format.
One codebase.
Multiple operating systems.
No nonsense.
```

---

## 2. Product Philosophy

DD-SSH should feel like a practical sysadmin tool, not a spaceship dashboard with 600 buttons.

The philosophy:

```text
Fast to open.
Fast to connect.
Easy to understand.
Easy to sync.
Easy to back up.
Hard to accidentally destroy things.
```

The user should be able to:

1. Open DD-SSH.
2. See a clean list of saved connections.
3. Double-click a server.
4. Get a real SSH terminal.
5. Work.

---

## 3. Core Feature Scope

Initial desired feature list:

```text
1. SSH terminal
2. Session manager
3. Password authentication
4. Private-key authentication
5. Tabs
6. Copy/paste
7. Export/import config/session data
8. Sync-friendly connection config
9. Keep-alive
10. Known-host handling
11. Multi-exec command
```

Explicitly excluded from early versions:

```text
- SFTP
- Split screen
- Built-in cloud account
- Built-in WebDAV sync in v1
- Android version
- Team sharing
- AI agent inside the application
- Automatic remote command execution without user confirmation
```

---

## 4. Technology Direction

Recommended initial stack:

```text
Language: C++
GUI framework: Qt 6
Build system: CMake
SSH library: libssh
Terminal frontend: xterm.js inside Qt WebEngine, at least for the first proof-of-concept
Config format: JSON
Version control: Git / GitHub
```

---

## 5. High-Level Architecture

```text
+------------------------------------------------------------+
|                          DD-SSH                            |
+------------------------------------------------------------+
| UI Layer                                                   |
| - MainWindow                                               |
| - Session sidebar                                          |
| - Terminal tabs                                            |
| - Settings dialog                                          |
| - Multi-exec panel                                         |
+------------------------------------------------------------+
| Core Layer                                                 |
| - SessionManager                                           |
| - ConfigManager                                            |
| - KnownHostsManager                                        |
| - MultiExecManager                                         |
| - SyncManager                                              |
+------------------------------------------------------------+
| SSH Layer                                                  |
| - SshSession                                               |
| - SshWorker                                                |
| - AuthManager                                              |
| - KeepAliveManager                                         |
+------------------------------------------------------------+
| Terminal Layer                                             |
| - TerminalTab                                              |
| - TerminalFrontend interface                               |
| - XtermJsTerminalFrontend                                  |
| - FutureNativeTerminalFrontend                             |
+------------------------------------------------------------+
```

---

## 6. Roadmap Summary

```text
v0.0 Planning and foundation
v0.1 SSH terminal proof
v0.2 Config and session manager
v0.3 Tabs and private keys
v0.4 Keep-alive and sync-friendly config
v0.5 Multi-exec
v0.6 Cross-platform build stabilization
v0.7 Packaging preparation
v1.0 First public release
```

---

## 7. Golden Rules

```text
1. Do not build everything at once.
2. Do not optimize before the first terminal works.
3. Do not add SFTP in v1.
4. Do not add split screen in v1.
5. Do not add AI features inside the app in v1.
6. Do not let UI classes contain SSH protocol logic.
7. Do not let SSH worker block the GUI thread.
8. Do not silently accept changed host keys.
9. Do not write secrets to logs.
10. Do not break cross-platform portability for short-term convenience.
11. Do not change config format without updating documentation.
12. Do not chase stores before the app is stable.
```

---

## 8. Immediate Next Mission

```text
Make one SSH terminal tab work correctly.
Then make it pleasant.
Then make it portable.
Then make it distributable.
```

This document is the foundation for DD-SSH and should live in:

```text
docs/PROJECT_BLUEPRINT.md
```
