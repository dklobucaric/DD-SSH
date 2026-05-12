# AGENTS.md

Guidelines for AI coding agents working on DD-SSH.

## Project

DD-SSH is a cross-platform SSH client and session manager.

## Stack

- C++
- Qt 6
- CMake
- libssh
- xterm.js / Qt WebEngine for early terminal frontend
- JSON config

## Rules

- Do not add Electron.
- Do not add telemetry.
- Do not store secrets in logs.
- Do not hardcode Linux-only, Windows-only, or macOS-only paths without abstraction.
- Keep SSH protocol logic outside UI classes.
- Keep terminal frontend abstracted.
- Keep config format documented in `docs/CONFIG_FORMAT.md`.
- Every feature must preserve Linux, Windows, and macOS portability.
- Prefer small focused classes.
- Do not modify `main` directly.
- Do not silently accept changed host keys.
- Do not introduce SFTP or split-screen into v1 scope unless explicitly requested.
