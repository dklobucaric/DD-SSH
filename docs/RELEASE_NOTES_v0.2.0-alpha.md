# DD-SSH v0.2.0-alpha — Andromeda

**Codename:** Andromeda  
**Milestone:** MF 0.2 — Real Terminal Foundation  
**Release type:** public alpha candidate notes

These notes are a draft for the first public alpha release. They should be reviewed and adjusted before creating a GitHub release/tag.

---

## Summary

DD-SSH is a clean cross-platform SSH client and session manager built with Qt/C++/CMake/libssh.

The Andromeda alpha focuses on the first real terminal foundation:

```text
saved session → known_hosts check → password/private-key auth → xterm.js terminal → PTY resize → shell work
```

It is usable for real SSH testing on Linux, but it is not a stable 1.0 release.

---

## Highlights

- Saved SSH sessions in one `dd-ssh.json` config file
- Password authentication
- Private-key authentication
- Known-host trust handling stored in the same config file
- Portable plaintext secrets mode: `secrets.mode = "plain-v1"`
- xterm.js terminal renderer bundled locally through Qt resources
- SSH PTY resize sync using xterm.js FitAddon
- Tested terminal programs: `htop`, `nano`, `vim`, `top`, `clear`
- Session create/edit/delete workflow
- Duplicate session warning for same `username@host:port`
- Reconnect after disconnect/reboot
- Config backups, import/export, restore latest backup
- Corrupt config recovery actions
- Settings foundation with app theme, terminal font size, and quick toolbar visibility

---

## Security warning

This alpha can store saved passwords and private keys in plaintext inside `dd-ssh.json`.

Do not publish, share, or commit your real `dd-ssh.json`.

Use only on trusted machines.

Encryption/master-password support is planned for a later version.

---

## Known limitations

- Linux is the primary tested platform so far
- Windows and macOS builds are not fully validated yet
- No encrypted secrets/master password yet
- No SSH agent integration yet
- No SFTP
- No split panes
- No Multi-Exec yet
- No official installers/packages yet
- No signed releases yet
- Terminal theme customization is intentionally deferred

---

## Suggested tester flow

1. Build DD-SSH from source.
2. Create one password session.
3. Create one private-key session.
4. Open each saved session with double-click.
5. Run:

```bash
whoami
hostname
stty size
htop
nano /tmp/dd-ssh-test.txt
vim /tmp/dd-ssh-test.txt
clear
```

6. Test disconnect/reconnect.
7. Export config.
8. Test corrupt config recovery on a copied config.
9. Report issues with OS, Qt version, libssh version, DD-SSH version, and steps to reproduce.

---

## Release checklist

Before tagging this alpha, complete:

- [ ] `docs/PUBLIC_ALPHA_CHECKLIST.md`
- [ ] `docs/TEST_MATRIX.md`
- [ ] README final review
- [ ] SECURITY_NOTES final review
- [ ] GitHub issue templates review
- [ ] Fresh clone build test

