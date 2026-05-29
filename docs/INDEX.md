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
- [Terminal Transport](TERMINAL_TRANSPORT.md) — SSH byte-stream terminal output/input path
- [File Transfer Architecture](FILE_TRANSFER_ARCHITECTURE.md) — planned libssh SFTP/File Manager design
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
- [Build/Test 0.1.8.5](BUILD_AND_TEST_0.1.8.5.md) — remote queue delete build/test notes
- [Build/Test 0.1.8.4](BUILD_AND_TEST_0.1.8.4.md) — overwrite metadata dialog build/test notes
- [Testcase 0.1.8.5](TESTCASE_0.1.8.5.md) — remote queue delete validation checklist
- [Testcase 0.1.8.4](TESTCASE_0.1.8.4.md) — overwrite metadata validation checklist
- [Build/Test 0.1.8.3](BUILD_AND_TEST_0.1.8.3.md) — file transfer logging/diagnostics build/test notes
- [Testcase 0.1.8.3](TESTCASE_0.1.8.3.md) — diagnostic logging validation checklist
- [Build/Test 0.1.8.2](BUILD_AND_TEST_0.1.8.2.md) — file manager safety polish build/test notes
- [Testcase 0.1.8.2](TESTCASE_0.1.8.2.md) — file manager safety validation checklist
- [Tester Checklist 0.1.8.1.2](TESTER_CHECKLIST_0.1.8.1.2.md) — third-party tester guide and bug-report format
- [Build/Test 0.1.8.1.2](BUILD_AND_TEST_0.1.8.1.2.md) — tester release polish build/test notes
- [Testcase 0.1.8.1.2](TESTCASE_0.1.8.1.2.md) — tester release polish validation checklist
- [Build/Test 0.1.8.1.1](BUILD_AND_TEST_0.1.8.1.1.md) — queue selected UI consolidation build/test notes
- [Testcase 0.1.8.1.1](TESTCASE_0.1.8.1.1.md) — queue selected UI consolidation validation checklist
- [Build/Test 0.1.8.1](BUILD_AND_TEST_0.1.8.1.md) — folder-transfer experiment build/test notes
- [Testcase 0.1.8.1](TESTCASE_0.1.8.1.md) — folder-transfer experiment validation checklist
- [Build/Test 0.1.8.0.4](BUILD_AND_TEST_0.1.8.0.4.md) — queue stabilization polish build/test notes
- [Build/Test 0.1.8.0.3](BUILD_AND_TEST_0.1.8.0.3.md) — queue retry-selected polish build/test notes
- [Build/Test 0.1.8.0.2](BUILD_AND_TEST_0.1.8.0.2.md) — queue overwrite-all polish build/test notes
- [Build/Test 0.1.8.0.1](BUILD_AND_TEST_0.1.8.0.1.md) — queue overwrite prompt polish build/test notes
- [Build/Test 0.1.8.0](BUILD_AND_TEST_0.1.8.0.md) — transfer queue foundation build/test notes
- [Build/Test 0.1.7.8](BUILD_AND_TEST_0.1.7.8.md) — transfer progress/cancel polish build/test notes
- [Build/Test 0.1.7.7](BUILD_AND_TEST_0.1.7.7.md) — single-file SFTP upload foundation build/test notes
- [Build/Test 0.1.7.6.1](BUILD_AND_TEST_0.1.7.6.1.md) — single-file SFTP download polish build/test notes
- [Testcase 0.1.8.0.4](TESTCASE_0.1.8.0.4.md) — queue stabilization polish validation checklist
- [Testcase 0.1.8.0.3](TESTCASE_0.1.8.0.3.md) — queue retry-selected polish validation checklist
- [Testcase 0.1.8.0.2](TESTCASE_0.1.8.0.2.md) — queue overwrite-all polish validation checklist
- [Testcase 0.1.8.0.1](TESTCASE_0.1.8.0.1.md) — queue overwrite prompt polish validation checklist
- [Testcase 0.1.8.0](TESTCASE_0.1.8.0.md) — transfer queue foundation validation checklist
- [Testcase 0.1.7.8](TESTCASE_0.1.7.8.md) — transfer progress/cancel polish validation checklist
- [Testcase 0.1.7.7](TESTCASE_0.1.7.7.md) — single-file SFTP upload foundation validation checklist
- [Testcase 0.1.7.6.1](TESTCASE_0.1.7.6.1.md) — single-file SFTP download polish validation checklist
- [Build/Test 0.1.7.6](BUILD_AND_TEST_0.1.7.6.md) — single-file SFTP download build/test notes
- [Testcase 0.1.7.6](TESTCASE_0.1.7.6.md) — single-file SFTP download validation checklist
- [Build/Test 0.1.7.4.1](BUILD_AND_TEST_0.1.7.4.1.md) — read-only SFTP browser bugfix polish build/test notes
- [Testcase 0.1.7.4.1](TESTCASE_0.1.7.4.1.md) — read-only SFTP browser bugfix validation checklist
- [Build/Test 0.1.7.4](BUILD_AND_TEST_0.1.7.4.md) — read-only remote file browser build/test notes
- [Testcase 0.1.7.4](TESTCASE_0.1.7.4.md) — read-only remote file browser validation checklist
- [Build/Test 0.1.7.3](BUILD_AND_TEST_0.1.7.3.md) — SFTP proof-of-concept build/test notes
- [Testcase 0.1.7.3](TESTCASE_0.1.7.3.md) — SFTP proof-of-concept validation checklist
- [Build/Test 0.1.6.5](BUILD_AND_TEST_0.1.6.5.md) — macOS DMG/dependency polish build notes
- [Testcase 0.1.6.5](TESTCASE_0.1.6.5.md) — macOS DMG/dependency validation checklist
- [Stabilization Checkpoint 0.1.5.9](STABILIZATION_CHECKPOINT_0.1.5.9.md) — validated stabilization summary

## Current checkpoint

```text
Version: dev 0.1.8.6
Codename: Andromeda
Milestone: File manager safety polish
Phase: Third-party tester preparation
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

- docs/TERMINAL_TRANSPORT.md explains the byte-stream terminal transport hardening added in dev 0.1.7.0.

- docs/FILE_TRANSFER_ARCHITECTURE.md explains the planned libssh SFTP/File Manager design and the dev 0.1.8.6 File Manager delete UI polish checkpoint.

- docs/TESTER_CHECKLIST_0.1.8.1.2.md explains how third-party testers should validate the current build and report issues.

- docs/BUILD_AND_TEST_0.1.8.0.4.1.md — Build and test notes for the queue stabilization compile hotfix.
- docs/TESTCASE_0.1.8.0.4.1.md — Testcase for the queue stabilization compile hotfix.
