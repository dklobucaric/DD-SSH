# DD-SSH Test Matrix

**Checkpoint:** dev 0.1.4.3 — Andromeda  
**Milestone:** MF 0.2 candidate — Real Terminal Foundation

This document tracks the manually confirmed test coverage for the current Andromeda development line.

The goal is not to pretend the app is finished. The goal is to make sure every important behavior that has been built so far has a clear manual test and an expected result before tagging a future `v0.2.0` milestone.

---

## Legend

```text
PASS      Confirmed working in manual testing
PARTIAL   Works, but needs polish or more coverage
TODO      Not implemented or not tested yet
```

---

## 1. Build and identity

| Area | Test | Expected result | Status |
|---|---|---|---|
| Build | `cmake --build build --clean-first` | Build completes and links `dd-ssh` | PASS |
| About dialog | Help → About DD-SSH | Shows version, codename, milestone, libssh version, and config path | PASS |
| Version | About dialog | Shows `dev 0.1.4.2` | PASS |
| Codename | About dialog | Shows `Andromeda` | PASS |
| Milestone | About dialog | Shows `MF 0.2 candidate` | PASS |

---

## 2. Config and saved sessions

| Area | Test | Expected result | Status |
|---|---|---|---|
| Config path | About dialog | Shows OS config path for `dd-ssh.json` | PASS |
| JSON load | Start app with existing config | Sidebar loads saved sessions from `dd-ssh.json` | PASS |
| JSON secrets | Saved password session | Password loads from `secrets.mode = plain-v1` and is not displayed | PASS |
| JSON secrets | Saved private-key session | Private key loads from `secrets.mode = plain-v1` and is not displayed | PASS |
| Known hosts | Reconnect to known host | Host is marked `TRUSTED` when fingerprint matches | PASS |
| Session create | Save successful manual connection | New session appears in sidebar | PASS |
| Session update | Save with same session ID | Existing session is updated instead of duplicated | PASS |
| Duplicate warning | Save same username + host + port | Offers Update existing / Create copy / Cancel | PASS |
| Session edit | Context menu → Edit session | Existing session can be modified | PASS |
| Session delete | Context menu → Delete session | Session is removed; known_hosts stays preserved | PASS |
| Settings save | Toolbar/Tools → Settings | Settings are saved under `settings` in `dd-ssh.json` | TODO |
| Terminal font settings | Change font size, open new terminal | Newly opened xterm.js tab uses the configured font size | TODO |
| Settings dialog layout | Open Settings from toolbar or Tools menu | Dialog opens at a readable size without manual resizing | PASS |

---

## 3. Authentication

| Area | Test | Expected result | Status |
|---|---|---|---|
| Password auth | Manual connection with password | Authentication succeeds | PASS |
| Private-key auth | Manual connection with key | Authentication succeeds | PASS |
| Saved password auth | Run auth test from saved session | Loads secret and authenticates successfully | PASS |
| Saved key auth | Run auth test from saved session | Loads key secret and authenticates successfully | PASS |
| Wrong password | Manual connection with wrong password | Auth fails and session is not saved as successful | PASS |
| Wrong/invalid key | Manual connection with bad key/path | Auth fails and session is not saved as successful | PASS |

---

## 4. xterm.js terminal renderer

| Area | Test | Expected result | Status |
|---|---|---|---|
| Default open | Double-click saved session | Opens xterm.js terminal tab | PASS |
| Local renderer | Terminal header | Shows `xterm.js ACTIVE - local bundled renderer` | PASS |
| Offline readiness | Open terminal without CDN dependency | Uses bundled Qt resource assets | PASS |
| Fallback | Local asset failure path | Fallback renderer remains available for debugging | PASS |
| Input | Type directly in terminal | Input reaches remote shell | PASS |
| Paste | Paste button / clipboard | Multiline input reaches remote shell | PASS |
| Ctrl+C | Run `ping 8.8.8.8`, send Ctrl+C | Remote command is interrupted | PASS |
| Clear | Run `clear` | Terminal clears correctly | PASS |
| Reset local terminal | Click Reset | Local xterm state resets without sending remote input | PASS |

---

## 5. PTY resize and terminal app behavior

| Area | Test | Expected result | Status |
|---|---|---|---|
| PTY size | Run `stty size` | Shows current remote rows/columns | PASS |
| Resize propagation | Resize DD-SSH window, run `stty size` again | Remote rows/columns change | PASS |
| `top` | Run `top` | Full-screen app renders and exits with `q` | PASS |
| `htop` | Run `htop` | Full-screen app renders and exits with `q`/F10 | PASS |
| `nano` | Run `nano /tmp/dd-ssh-test.txt` | Editor opens; Ctrl shortcuts work | PASS |
| `vim` | Run `vim /tmp/dd-ssh-test.txt` | Editor opens and can be exited normally | PASS |
| ANSI colors | Run `ls -la` | Colors render without raw escape-sequence garbage | PASS |

---

## 6. Terminal lifecycle

| Area | Test | Expected result | Status |
|---|---|---|---|
| Active tab close | Close tab while SSH shell is connected | App asks before disconnecting and closing | PASS |
| User disconnect | Click Disconnect | Shell closes; controls update to disconnected state | PASS |
| Remote exit | Run `exit` | Terminal marks session disconnected | PASS |
| Remote reboot | Reboot server from another terminal | DD-SSH shows broadcast/error, cleans up, marks disconnected | PASS |
| Reconnect | Click Reconnect after disconnect | Same tab starts a new SSH shell session | PASS |
| Reconnect guard | Session is connected | Reconnect remains disabled | PASS |
| Input after disconnect | Session is disconnected | Remote input actions are disabled | PASS |

---

## 7. Current known limitations

These are expected at this stage and should not be treated as regressions unless a previous confirmed behavior breaks.

```text
- Terminal UI still has visible dev/debug controls.
- Settings dialog is still a placeholder.
- Multi-Exec is still a placeholder.
- Plaintext secrets are intentional for early portable config work but are insecure.
- No encrypted secrets/master password yet.
- No import/export UI yet.
- No custom config path / portable mode UI yet.
- No keep-alive settings yet.
- No packaging/installers yet.
```

---

## 8. MF 0.2 candidate checklist

Before tagging a future `v0.2.0 — Andromeda`, confirm:

```text
[ ] Build from clean source
[ ] Password saved session opens xterm terminal
[ ] Private-key saved session opens xterm terminal
[ ] local xterm.js renderer is active
[ ] stty size changes after window resize
[ ] htop works
[ ] nano works
[ ] vim works
[ ] top works
[ ] clear works
[ ] Ctrl+C interrupts ping
[ ] remote reboot is detected cleanly
[ ] reconnect works after disconnect
[ ] active tab close asks for confirmation
[ ] README/CHANGELOG/TEST_MATRIX are updated
```

When these are all confirmed, DD-SSH has a strong claim to the **MF 0.2 — Real Terminal Foundation** milestone.
