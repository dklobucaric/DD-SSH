# DD-SSH Use Cases

This document explains what DD-SSH is useful for today and how each use case maps to current features.

## Use case 1: Everyday SSH client

**Goal:** Open a saved server quickly and work in a normal terminal.

Workflow:

```text
1. Save the server as a session.
2. Double-click it in the sidebar.
3. Work in the xterm.js terminal.
```

Useful commands:

```bash
whoami
hostname
uptime
df -h
htop
journalctl --no-pager -n 50
```

Current support: **works**.

## Use case 2: Personal portable SSH profile

**Goal:** Move one file to another trusted computer and keep sessions, settings, known_hosts, and secrets.

Workflow:

```text
File → Export Config...
Copy exported JSON to trusted machine
File → Import Config...
```

Important: the config may contain plaintext passwords and private keys.

Current support: **works with plaintext secrets**.

## Use case 3: Password session

**Goal:** Save and open a server that uses password authentication.

Workflow:

```text
Session → New Session
Auth type: Password
Save after successful auth
Double-click session
```

Current support: **works**.

## Use case 4: Private-key session

**Goal:** Save and open a server that uses private-key authentication.

Workflow:

```text
Session → New Session
Auth type: Private key
Select key file
Save after successful auth
Double-click session
```

Current support: **works**.

Implementation note: early builds can store the private key content in plaintext under `secrets.items`.

## Use case 5: Full-screen terminal app

**Goal:** Run interactive terminal apps like `htop`, `nano`, or `vim`.

Workflow:

```bash
htop
nano /tmp/dd-ssh-test.txt
vim /tmp/dd-ssh-test.txt
```

Current support: **works in Andromeda tests** with local xterm.js + PTY resize.

## Use case 6: Remote server reboot/disconnect

**Goal:** DD-SSH should not freeze or spin forever when the remote host disappears.

Workflow:

```text
1. Open DD-SSH terminal.
2. Reboot server from another SSH connection.
3. Watch DD-SSH detect disconnect.
4. Click Reconnect after server returns.
```

Current support: **works**.

## Use case 7: Config recovery

**Goal:** Recover from invalid `dd-ssh.json` caused by manual edits or sync conflict.

Workflow:

```text
DD-SSH detects invalid config
Recovery dialog opens
Choose restore latest backup or create fresh config
```

Current support: **works**.

## Use case 8: Public alpha testing

**Goal:** Validate DD-SSH behavior on real machines.

Run through:

- Build test
- Session create/edit/delete
- Password auth
- Private-key auth
- Terminal app tests
- Disconnect/reconnect tests
- Config export/import/recovery tests

Use [TEST_MATRIX.md](TEST_MATRIX.md).

## Use case 9: Future Multi-Exec

**Goal:** Send the same command to multiple open SSH tabs.

Example future workflow:

```text
Open 3 server tabs
Tools → Multi-Exec
Command: uptime
Targets: selected tabs
Send text only or send + Enter
```

Current support: **not implemented**. The menu item is a placeholder.

This feature is powerful and risky. It needs target preview, safe send modes, dangerous command warnings, and logging.
