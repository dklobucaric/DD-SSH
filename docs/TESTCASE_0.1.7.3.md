# Testcase — dev 0.1.7.3

**Checkpoint:** dev 0.1.7.3 — Andromeda  
**Feature:** SFTP connection proof of concept

---

## Purpose

Validate that DD-SSH can use a saved session to open a real libssh SFTP subsystem and list a remote directory, while preserving the tested terminal foundation from dev 0.1.7.1.

This is not a file-manager UI test yet. It is a transport proof.

---

## Preconditions

Use a saved DD-SSH session that already passes normal terminal testing.

Recommended saved-session types:

```text
[ ] one password-auth session
[ ] one private-key-auth session
```

The remote SSH server should allow SFTP subsystem access.

---

## Build identity

```text
[ ] Build completes
[ ] App starts
[ ] Help → About DD-SSH shows Version: dev 0.1.7.3
[ ] Welcome tab mentions SFTP connection proof of concept
```

---

## Main SFTP probe test

1. Start DD-SSH.
2. Right-click a saved session in the sidebar.
3. Click `Open File Manager (SFTP probe)`.
4. Follow the known-host flow.
5. Confirm a new text tab opens.

Expected result:

```text
[ ] result tab title includes "sftp"
[ ] output starts with "DD-SSH SFTP connection proof of concept"
[ ] SSH preflight result is shown
[ ] known-host decision is shown
[ ] host-key verification before auth is shown when expectation is enabled
[ ] SFTP probe result is shown
[ ] successful server shows "Status: SUCCESS"
[ ] successful server shows remote path "."
[ ] successful server shows entry count
[ ] successful server shows a simple remote listing table
```

---

## Password-auth SFTP probe

```text
[ ] password-auth saved session loads its secret without displaying the password
[ ] SSH preflight succeeds
[ ] known-host decision allows continuing
[ ] SFTP connection verifies approved host key before authentication
[ ] authentication succeeds
[ ] SFTP subsystem initializes
[ ] remote `.` listing appears
```

---

## Private-key SFTP probe

```text
[ ] private-key saved session loads its secret without displaying private-key contents
[ ] temporary key file is used internally only
[ ] SSH preflight succeeds
[ ] known-host decision allows continuing
[ ] SFTP connection verifies approved host key before authentication
[ ] authentication succeeds
[ ] SFTP subsystem initializes
[ ] remote `.` listing appears
```

---

## Negative / safe failure checks

Use only safe tests.

```text
[ ] wrong password/private key fails cleanly and does not crash
[ ] server without SFTP support fails cleanly with SFTP init/open error
[ ] permission problem while listing remote `.` fails cleanly if applicable
[ ] cancelling known-host prompt stops before authentication
[ ] handshake failure stops before known-host/auth/SFTP
```

Expected behavior:

```text
[ ] result tab explains where the flow stopped
[ ] no password/private-key value is displayed
[ ] no upload/download/delete action appears
[ ] app remains usable after failure
```

---

## Terminal regression

After running the SFTP probe:

```text
[ ] double-click saved session still opens xterm.js terminal
[ ] `whoami` works
[ ] `pwd` works
[ ] `clear` works
[ ] fullscreen apps still work where previously validated: htop/top/nano/vim
[ ] toolbar Paste works
[ ] right-click paste works
[ ] Ctrl+Shift+V / Command+V works
[ ] Ctrl+C interrupts a remote process
[ ] Session Traffic updates for terminal tabs
```

---

## Diagnostic logging privacy

With diagnostic logging enabled:

```text
[ ] log may mention SFTP probe start/success/failure
[ ] log may mention host/port/user/method
[ ] log may mention entry count
[ ] log does not contain password values
[ ] log does not contain private-key contents
[ ] log does not contain terminal input/output
[ ] log does not contain clipboard content
[ ] log does not contain file contents
[ ] log does not contain full dd-ssh.json
```

---

## Pass criteria

`dev 0.1.7.3` passes when:

```text
[ ] Linux build succeeds
[ ] About shows dev 0.1.7.3
[ ] SFTP probe succeeds on at least one known-good saved session
[ ] terminal baseline still works after the probe
[ ] no secrets are displayed/logged
[ ] no file transfer UI/actions exist yet
```
