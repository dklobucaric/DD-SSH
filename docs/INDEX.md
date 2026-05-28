# DD-SSH Documentation Index

This directory contains the working documentation for DD-SSH.

## User-facing docs

- [Getting Started](GETTING_STARTED.md) — first build, first session, first terminal
- [User Guide](USER_GUIDE.md) — menus, workflows, sessions, settings, terminal use
- [Use Cases](USE_CASES.md) — realistic ways DD-SSH is expected to be used
- [Screenshots](SCREENSHOTS.md) — visual overview of the installed DD-SSH app
- [Features and Limitations](FEATURES.md) — what works now, what does not
- [Troubleshooting](TROUBLESHOOTING.md) — common problems and recovery steps

## Technical docs

- [Architecture](ARCHITECTURE.md) — UI/core/SSH/terminal/config layers
- [Config Format](CONFIG_FORMAT.md) — `dd-ssh.json` structure
- [Config Management](CONFIG_MANAGEMENT.md) — backups, import/export, recovery
- [Security Notes](SECURITY_NOTES.md) — plaintext secrets, known_hosts, future encryption
- [Building](BUILDING.md) — local build instructions
- [Windows Build Guide](WINDOWS_BUILD.md) — native Windows MSVC/Qt/vcpkg build notes
- [Windows Deployment Guide](WINDOWS_DEPLOYMENT.md)
- [Linux Packaging Guide](LINUX_PACKAGING.md) — Linux packaging notes and first `.deb` package path
- [Debian Package Tutorial](DEBIAN_PACKAGE_TUTORIAL.md) — step-by-step `.deb` build/install/remove tutorial
- [macOS Build Guide](MACOS_BUILD.md) — Intel macOS source build notes
- [macOS Deployment Guide](MACOS_DEPLOYMENT.md) — `.app` / `.dmg` deployment workflow
- [Windows libssh Handshake Compatibility Test](WINDOWS_LIBSSH_HANDSHAKE_COMPATIBILITY.md) — Windows KEX compatibility regression notes
- [Packaging](PACKAGING.md) — future distribution notes

## Project management docs

- [Project Blueprint](PROJECT_BLUEPRINT.md) — product direction
- [Roadmap](ROADMAP.md) — version plan and future features
- [Test Matrix](TEST_MATRIX.md) — manual validation checklist
- [Changelog](CHANGELOG.md) — checkpoint history
- [Release Process](RELEASE_PROCESS.md)
- [Release Artifacts](RELEASE_ARTIFACTS.md)
- [Release Notes Template](RELEASE_NOTES_TEMPLATE.md) — future release checklist
- [Release Checklist](RELEASE_CHECKLIST.md) — focused checkpoint smoke-test checklist
- [Build/Test 0.1.6.5](BUILD_AND_TEST_0.1.6.5.md) — macOS DMG/dependency polish build notes
- [Testcase 0.1.6.5](TESTCASE_0.1.6.5.md) — macOS DMG/dependency validation checklist
- [Stabilization Checkpoint 0.1.5.9](STABILIZATION_CHECKPOINT_0.1.5.9.md) — validated stabilization summary

## Current checkpoint

```text
Version: dev 0.1.6.9
Codename: Andromeda
Milestone: MF 0.2 candidate
Phase: macOS DMG/dependency polish
```


## Public alpha preparation

- [Public Alpha Checklist](PUBLIC_ALPHA_CHECKLIST.md)
- [Release Notes Draft: v0.2.0-alpha](RELEASE_NOTES_v0.2.0-alpha.md)
- [Windows Build Guide](WINDOWS_BUILD.md)
- [Windows Deployment Guide](WINDOWS_DEPLOYMENT.md)
- [Linux Packaging Guide](LINUX_PACKAGING.md)
- [Debian Package Tutorial](DEBIAN_PACKAGE_TUTORIAL.md)
- [macOS Build Guide](MACOS_BUILD.md)
- [macOS Deployment Guide](MACOS_DEPLOYMENT.md)
- [Screenshots](SCREENSHOTS.md)
- [Known Limitations](KNOWN_LIMITATIONS.md)
- [Stabilization Checkpoint 0.1.5.9](STABILIZATION_CHECKPOINT_0.1.5.9.md)
- [Release Checklist](RELEASE_CHECKLIST.md)

GitHub issue templates live under `.github/ISSUE_TEMPLATE/`.

- [Windows libssh handshake compatibility test](WINDOWS_LIBSSH_HANDSHAKE_COMPATIBILITY.md)

- `LOGGING.md` — optional diagnostic logging, log folders, and secret-safety rules.

- docs/SESSION_TRAFFIC.md explains the status-bar Session Traffic monitor added in dev 0.1.6.7.

- docs/CONFIG_IMPORT_EXPORT_SAFETY.md explains the config import/export preview added in dev 0.1.6.8.
