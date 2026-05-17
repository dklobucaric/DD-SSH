# DD-SSH Public Alpha Checklist

**Checkpoint:** dev 0.1.5.0 — Andromeda  
**Milestone target:** v0.2.0-alpha — Real Terminal Foundation

This checklist is the final pre-alpha gate. It is intentionally practical: run it on a real machine, with real saved sessions, before tagging a public alpha.

---

## 1. Build identity

- [ ] Clean build succeeds:

```bash
cmake --build build --clean-first
```

- [ ] App launches:

```bash
./build/dd-ssh
```

- [ ] Help → About DD-SSH shows:

```text
Version: dev 0.1.5.0
Codename: Andromeda
Milestone: MF 0.2 candidate
Current phase: Public alpha release preparation.
```

- [ ] Welcome tab opens and describes the current Andromeda status.

---

## 2. Config path and safety

- [ ] About dialog shows the Linux config path:

```text
~/.config/DD-LAB/DD-SSH/dd-ssh.json
```

- [ ] `dd-ssh.json` is valid JSON:

```bash
jq . ~/.config/DD-LAB/DD-SSH/dd-ssh.json >/dev/null && echo "JSON OK"
```

- [ ] Config file permissions are owner-readable/writable where supported:

```bash
stat -c '%a %n' ~/.config/DD-LAB/DD-SSH/dd-ssh.json
```

Preferred result on Linux:

```text
600 .../dd-ssh.json
```

- [ ] Settings save creates a backup file when backups are enabled.
- [ ] Backup rotation does not delete the active config.

---

## 3. Saved session workflow

- [ ] Session → New Session creates a saved password session.
- [ ] Session → New Session creates a saved private-key session.
- [ ] Double-clicking a saved session opens an xterm.js terminal.
- [ ] Session → Connect / Auth test still works for manual testing.
- [ ] Session → Edit selected session updates name/group/host/auth fields correctly.
- [ ] Deleting a session removes the session and orphan secret but preserves known_hosts.
- [ ] Duplicate username + host + port warning offers Update existing / Create copy / Cancel.

---

## 4. Terminal behavior

Run these in a password session and a private-key session:

```bash
whoami
hostname
pwd
stty size
ls -la
clear
htop
nano /tmp/dd-ssh-alpha-test.txt
vim /tmp/dd-ssh-alpha-test.txt
top
exit
```

Expected:

- [ ] xterm.js local renderer is active.
- [ ] No CDN is required for xterm.js.
- [ ] ANSI color output renders correctly.
- [ ] `stty size` follows window resize.
- [ ] `htop`, `nano`, `vim`, `top`, and `clear` are usable.
- [ ] `exit` marks the tab disconnected.

---

## 5. Lifecycle behavior

- [ ] Disconnect button closes the shell cleanly.
- [ ] Reconnect button reconnects a disconnected tab.
- [ ] Closing an active tab asks for confirmation.
- [ ] Closing a disconnected tab does not show an active-session warning.
- [ ] External server reboot/disconnect is detected and cleaned up.
- [ ] After disconnect, remote input actions are disabled until reconnect.

---

## 6. Settings and appearance

- [ ] Settings opens from Tools → Settings.
- [ ] App theme System / Light / Dark works and persists.
- [ ] Terminal font size applies to newly opened terminal tabs.
- [ ] Optional quick action toolbar can be shown/hidden and persists.
- [ ] Plaintext secrets warning is visible in Settings.
- [ ] Open Config Folder opens the expected directory.

---

## 7. Config import/export/restore

- [ ] File → Export Config creates a valid JSON export.
- [ ] File → Import Config rejects invalid JSON.
- [ ] File → Import Config accepts valid exported config after confirmation.
- [ ] Import creates a backup of the previous active config.
- [ ] File → Restore Latest Backup restores a valid backup after confirmation.
- [ ] App reloads sessions/settings after import/restore.

---

## 8. Corrupt config recovery

Create a temporary backup first:

```bash
cp ~/.config/DD-LAB/DD-SSH/dd-ssh.json ~/.config/DD-LAB/DD-SSH/dd-ssh.json.good
```

Then corrupt the active file:

```bash
printf '{ invalid json\n' > ~/.config/DD-LAB/DD-SSH/dd-ssh.json
```

Expected:

- [ ] Startup shows a config recovery warning.
- [ ] Corrupt file is not overwritten automatically.
- [ ] Continue read-only opens the app without loading sessions.
- [ ] Restore latest valid backup moves corrupt config aside and restores backup.
- [ ] Create fresh config moves corrupt config aside and creates a default config.

Restore your real test config when finished:

```bash
cp ~/.config/DD-LAB/DD-SSH/dd-ssh.json.good ~/.config/DD-LAB/DD-SSH/dd-ssh.json
chmod 600 ~/.config/DD-LAB/DD-SSH/dd-ssh.json
```

---

## 9. Documentation and public alpha readiness

- [ ] README describes current status and limitations.
- [ ] SECURITY_NOTES clearly warns about plaintext secrets.
- [ ] TEST_MATRIX is conservative and current.
- [ ] RELEASE_NOTES_v0.2.0-alpha.md is ready.
- [ ] GitHub issue templates exist.
- [ ] Known limitations are not hidden.

---

## 10. Final decision

Before tagging `v0.2.0-alpha`, answer honestly:

```text
Can an external tester build DD-SSH, create a session, open xterm.js terminal, recover from config trouble, and understand the plaintext secret warning?
```

If yes, Andromeda is ready for public alpha.
