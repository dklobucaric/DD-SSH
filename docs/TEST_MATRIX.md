# DD-SSH Test Matrix

**Checkpoint:** dev 0.1.5.8 — Andromeda
**Milestone:** MF 0.2 candidate — Real Terminal Foundation
**Phase:** Windows libssh handshake compatibility polish

This matrix tracks what has been confirmed manually, what is implemented but should be re-tested before a public alpha tag, and what is still planned.

## Legend

```text
PASS              Confirmed working in manual testing.
IMPLEMENTED       Feature exists, but should be re-tested before public alpha tagging.
PARTIAL           Works, but needs polish or broader testing.
NOT TESTED        Important scenario not yet manually verified.
TODO              Not implemented yet.
```

## 1. Build and identity

| Area | Test | Expected result | Status | Notes |
|---|---|---|---|---|
| Build | `cmake --build build --clean-first` | Build completes and links `dd-ssh` | PASS | Re-tested frequently during development. |
| Launch | `./build/dd-ssh` | App opens | PASS | Linux primary test platform. |
| Windows configure | CMake with MSVC/Qt/vcpkg/pkgconf | Configure completes | PASS | Confirmed on native Windows branch. |
| Windows Debug build | `cmake --build build-win` | `dd-ssh.exe` builds | PASS | Confirmed on native Windows. |
| Windows launch | `build-win\dd-ssh.exe` with Qt/vcpkg DLL paths | App opens | PASS | Confirmed; Welcome screen and UI visible. |
| Windows Release build | `build-win-release` | Release exe builds | PASS | Confirmed on native Windows before the deployment pass. |
| Windows libssh KEX compatibility | OpenSSH 10 server advertising ML-KEM/SNTRUP KEX first | DD-SSH reaches auth/shell flow on Windows 10/11 | PENDING | Regression target: `lab.dd-lab.hr:2231`; server-side KEX workaround proved root cause before app-side fix. |
| Windows deployment script | `scripts\windows-deploy-release.bat` after Release build | Creates `dist\windows-release` and starts deployed exe | PASS | Simple working BAT from the successful Windows standalone deployment test is now checked in. |
| Windows deployed launch | `dist\windows-release\dd-ssh.exe` from normal Command Prompt | App starts without Qt/vcpkg PATH | PASS | Confirmed on real Windows 10/11 machines during standalone deployment testing. |
| Clean Windows 10 deploy-folder test | Copy `dist\windows-release` to a clean Windows 10 machine | App launches without dev tools installed | PASS | App launches, icon appears, import/connect works, and xterm terminal is fast. |
| About | Help → About DD-SSH | Shows version/codename/milestone/config path | PASS | Verified after version/codename work. |
| Version | About dialog | Shows current checkpoint version | PASS | Should be checked after every patch. |
| Codename | About dialog | Shows `Andromeda` | PASS | Current 0.1.x codename line. |
| Welcome screen | First tab on startup | Shows current status/dashboard, not old skeleton text | PASS | Updated during docs/welcome polish. |
| Terminal startup notice | New terminal tab displays loading/startup state before xterm.js bridge is ready | User sees startup message instead of blank terminal during first WebEngine initialization | IMPLEMENTED | Added in dev 0.1.5.3; especially useful on Windows where first Qt WebEngine startup can be slower. |
| App icon resources | Build includes Qt/Windows/macOS/Linux icon assets | Window/exe/bundle icon resources exist | IMPLEMENTED | Added in 0.1.5.2; needs visual verification on Windows taskbar/Explorer and future Linux/macOS packaging. |

## 2. Sessions and menus

| Area | Test | Expected result | Status | Notes |
|---|---|---|---|---|
| Load sessions | Start app with valid config | Sidebar loads saved sessions | PASS | Tested repeatedly with real saved sessions. |
| New session | Session → New Session | Creates saved session after successful auth | PASS | Uses plaintext portable secret storage. |
| Failed new session save | Attempt New Session with failed auth | Session is not saved | IMPLEMENTED | Documented in dev 0.1.5.5; final pass should confirm wrong password/port does not create a sidebar entry. |
| Manual auth | Session → Connect / Auth test | Runs auth test; save optional | PASS | Old auth-test flow preserved. |
| Edit session | Session → Edit selected session / context menu | Edits selected saved session | PASS | Password/key can be kept or replaced. |
| Delete session | Context menu → Delete | Deletes session, preserves known_hosts | PASS | Orphan secret cleanup implemented. |
| Duplicate warning | Save same username+host+port | Offers update/copy/cancel | PASS | Manual save polish confirmed earlier. |
| Double-click | Double-click saved session | Opens xterm.js terminal | PASS | Default behavior since 0.1.3.4. |
| File menu | File menu | Config-level actions and Exit live here | IMPLEMENTED | Re-test after import/export and toolbar changes. |
| Session menu | Session menu | New/Connect/Edit session actions live here | PASS | Polished in 0.1.4.6. |
| Quick toolbar | Settings checkbox | Toolbar can be shown/hidden and persists | IMPLEMENTED | Needs final visual re-test after revised sizing patch. |

## 3. Auth and known_hosts

| Area | Test | Expected result | Status | Notes |
|---|---|---|---|---|
| Password auth | Saved password session | Auth succeeds | PASS | Tested with real LAN host. |
| Private-key auth | Saved key session | Auth succeeds | PASS | Tested with embedded plaintext private key from JSON. |
| Known host | Reconnect to same host | TRUSTED when fingerprint matches | PASS | Confirmed after known_hosts save/load. |
| Unknown host | First connect to new host | Prompts/trust flow works | PASS | Confirmed during first-connect tests. |
| Multi-key known host A | ED25519-only config on Windows 10 where libssh negotiates ECDSA | Offers Trust additional key, connects, saves both keys | TODO | Regression case: 138.2.166.222:223. |
| Multi-key known host B | ECDSA-only config on Linux/Windows 11 where libssh negotiates ED25519 | Offers Trust additional key, connects, saves both keys | TODO | Reverse regression case for portable JSON. |
| Trust once additional key | Additional key type prompt → Trust once | Opens shell/auth flow without saving JSON | TODO | Confirms transient trust path. |
| Changed host | Same key type with different fingerprint | Must show strong host-key-changed warning | TODO | Needs deliberate edited-copy config test before stable release. |

## 4. Terminal renderer and shell behavior

| Area | Test | Expected result | Status | Notes |
|---|---|---|---|---|
| Renderer | Open terminal | Header says xterm.js ACTIVE local bundled renderer | PASS | Confirmed after Qt resource path fix. |
| Offline/local assets | Open terminal without CDN dependency | xterm.js still loads | PASS | Local bundled renderer confirmed active. |
| Basic command | `whoami` / `hostname` / `pwd` | Output appears | PASS | Confirmed. |
| Paste | Paste multiline commands | Commands execute | PASS | Fixed and confirmed. |
| Ctrl+C | `ping 8.8.8.8`, Ctrl+C | Ping stops | PASS | Confirmed. |
| PTY resize | `stty size`, resize window | rows/cols change | PASS | Confirmed with multiple window sizes. |
| htop | `htop` | Full-screen app renders | PASS | Confirmed. |
| nano | `nano /tmp/dd-ssh-test.txt` | Opens, text entry and Ctrl commands work | PASS | Confirmed, including Ctrl shortcuts. |
| vim | `vim /tmp/dd-ssh-test.txt` | Opens and works enough for manual testing | PASS | Confirmed. |
| top | `top` | Opens and updates | PASS | Confirmed. |
| clear | `clear` | Clears terminal | PASS | Confirmed. |
| Multiple terminal tabs | Open more than one saved session terminal | Inputs stay with correct session | PASS | Confirmed earlier with parallel tab testing. |

## 5. Terminal lifecycle

| Area | Test | Expected result | Status | Notes |
|---|---|---|---|---|
| Exit shell | `exit` | Terminal becomes disconnected | PASS | Confirmed. |
| Disconnect button | Click Disconnect | Session closes and controls update | PASS | Confirmed. |
| Remote reboot | Reboot server externally | DD-SSH detects disconnect and cleans up | PASS | Confirmed with real reboot test. |
| Reconnect | Click Reconnect after disconnect | Same tab reconnects using saved session | PASS | Confirmed after 0.1.3.7. |
| Close active tab | Click tab close while connected | Confirms before disconnecting | PASS | Confirmed in lifecycle polish. |
| Close active app window | Window close button / X while connected tabs exist | Confirms before disconnecting all active sessions and exiting | IMPLEMENTED | Added in dev 0.1.5.5; confirmed during deployed-folder testing. |
| File Exit with active sessions | `File → Exit` while connected tabs exist | Uses the same active-session confirmation as window close | IMPLEMENTED | Added in dev 0.1.5.5 via main window close event; include this in deployed-folder testing. |
| Close disconnected tab | Close after disconnect | Closes without active-session warning | IMPLEMENTED | Should be included in next final pass. |

## 6. Settings and app UI

| Area | Test | Expected result | Status | Notes |
|---|---|---|---|---|
| Open Settings | Tools → Settings | Dialog opens readable | PASS | Sizing polish added after visual issue. |
| App theme | System/Light/Dark | Qt app theme changes after OK and persists | PASS | Confirmed by user; app theme changes apply and persist. |
| Terminal font | Change font size | New terminal tabs use new font | PASS | Confirmed by user. |
| Backup setting | Enable/disable backups, max count | Setting is saved | PASS | Confirmed together with backup creation. |
| Open config folder | Settings/File action | Opens config folder | PASS | Confirmed during Windows/AppData validation path and Linux settings workflow. |
| Plaintext warning | Settings dialog | Orange plaintext secrets warning visible | PASS | Confirmed and intentionally kept. |
| Dark terminal | xterm terminal | Terminal remains dark independent of app theme | PASS | Intentional for now; xterm theming deferred. |

## 7. Config safety and recovery

| Area | Test | Expected result | Status | Notes |
|---|---|---|---|---|
| Default config | First run/no config file | Creates or uses healthy default config structure | IMPLEMENTED | Should be explicitly re-tested by moving config folder aside. |
| Backup before save | Save settings/session | `dd-ssh.json.bak-*` created | PASS | Confirmed. |
| Backup rotation | More backups than max count | Old backups are pruned | IMPLEMENTED | Needs explicit stress test. |
| Corrupt startup | Start with invalid config | Recovery dialog appears; file not overwritten | PASS | Confirmed. |
| Continue read-only | Recovery dialog | App opens without overwriting corrupt config | PASS | Confirmed in recovery flow. |
| Restore latest valid backup | Recovery dialog | Corrupt file moved aside, latest valid backup restored | PASS | Confirmed as part of recovery-actions testing if completed. |
| Create fresh config | Recovery dialog | Corrupt file moved aside, default config created | PASS | Confirmed as part of recovery-actions testing if completed. |
| Export config | File → Export Config | Valid JSON copied to chosen path | IMPLEMENTED | Needs explicit final public-alpha pass if not already tested. |
| Import invalid | Import invalid JSON | Import refused, active config unchanged | IMPLEMENTED | Needs explicit final public-alpha pass if not already tested. |
| Import valid | Import exported config | Pre-import backup created and config reloads | IMPLEMENTED | Needs explicit final public-alpha pass if not already tested. |
| Restore latest from File | File → Restore Latest Backup | Latest valid backup restored | IMPLEMENTED | Needs explicit final public-alpha pass if not already tested. |

## 8. Documentation

| Requirement | Status | Notes |
|---|---|---|
| README current | PASS | Public-alpha style README added. |
| Security warning visible | PASS | Plaintext secrets warning documented. |
| Config docs current | PASS | CONFIG_FORMAT updated. |
| Test matrix current | PASS | This file should stay conservative: do not mark untested items PASS. |
| Changelog current | PASS | CHANGELOG tracks development checkpoints. |
| Public alpha checklist | PASS | PUBLIC_ALPHA_CHECKLIST documents final pre-alpha gate. |
| Release notes draft | PASS | RELEASE_NOTES_v0.2.0-alpha.md added for alpha tagging. |
| GitHub issue templates | PASS | Bug, terminal, config/recovery, and feature request templates added. |
| Use cases documented | PASS | USE_CASES added. |
| Troubleshooting documented | PASS | TROUBLESHOOTING added. |
| Known limitations documented | PASS | Public alpha limitations documented. |
| Packaging docs | PARTIAL | Still planning-level, not release-proven. |

## 9. Platform coverage

| Platform | Status | Notes |
|---|---|---|
| Linux | PASS | Primary tested platform. |
| Windows | PARTIAL | Native Windows Debug/Release builds launch and SSH/xterm/htop work. Deploy-folder and installer validation are not complete yet. |
| macOS | TODO | Build/runtime not validated yet. |


## 9a. Windows validation details

| Area | Test | Expected result | Status | Notes |
|---|---|---|---|---|
| MSVC | `cl` in x64 Native Tools prompt | Microsoft C/C++ compiler is available | PASS | MSVC x64 confirmed. |
| Qt WebEngine | Qt 6.11.1 MSVC 2022 64-bit | WebEngineWidgets found by CMake | PASS | Needed Qt Positioning dependency. |
| Qt WebChannel | `Qt6WebChannelConfig.cmake` exists | WebChannel bridge available | PASS | Confirmed in Qt install. |
| Qt Positioning | `Qt6PositioningConfig.cmake` exists | WebEngine dependency resolves | PASS | Added after first CMake failure. |
| vcpkg libssh | `vcpkg install libssh:x64-windows` | libssh installed | PASS | vcpkg reported successful install. |
| pkgconf | vcpkg `pkgconf` path passed to CMake | `pkg_check_modules(libssh)` works | PASS | CMake found libssh 0.12.0. |
| AppData config | First Windows launch | Config stored under AppData/Local/DD-LAB/DD-SSH | PASS | Windows path verified by app launch behavior. |
| Windows terminal | Saved session opens xterm | SSH terminal opens | PASS | Confirmed with real SSH session. |
| Windows htop | `htop` in terminal | Fullscreen terminal app renders | PASS | Confirmed by screenshot/test. |
| Windows startup/RAM | Task Manager observation | Record initial metrics | PARTIAL | Debug build showed several-second first terminal startup and ~350–380 MB RAM with WebEngine terminal. Release-build measurement should be recorded during deployment testing. |
| Windows deploy | Run outside build environment | App starts without developer PATH | PASS | Confirmed on real Windows 10/11 machines. |

## 10. Public alpha readiness summary

| Requirement | Status | Notes |
|---|---|---|
| Core saved-session workflow | PASS | Create/edit/delete/open terminal tested. |
| Real terminal foundation | PASS | xterm.js, PTY resize, fullscreen apps tested. |
| Config safety foundation | PASS | Backup and corrupt config recovery exist. |
| Config import/export | IMPLEMENTED | Needs final focused pass before public alpha tag. |
| Settings foundation | PASS | Font/app settings exist; theme needs final confirmation if not already done. |
| Security posture documented | PASS | Plaintext secrets warning exists. |
| Windows/macOS validation | PARTIAL | Windows native Debug and Release builds are validated; Windows deploy-folder and macOS remain pending. |
| Packaging/installers | TODO | Not part of current dev line. |

## 11. Suggested final pre-alpha test pass

Before tagging an Andromeda public alpha, run this short pass:

```bash
cmake --build build --clean-first
./build/dd-ssh
```

Then verify:

```text
1. About shows the expected version/codename/milestone.
2. Saved password session opens xterm terminal.
3. Saved key session opens xterm terminal.
4. whoami, htop, nano, vim, top, clear work.
5. stty size changes after resizing the window.
6. Disconnect and Reconnect work.
7. Settings save and persist.
8. Config backup is created before save.
9. Corrupt config recovery still works.
10. Export/import/restore config actions pass a focused test.
11. Windows Release build test records startup/RAM notes.
```
