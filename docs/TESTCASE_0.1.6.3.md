# DD-SSH dev 0.1.6.3 — Testcase

## Purpose

Verify that DD-SSH does not send a password or private key until the real authentication/shell connection has verified the same SSH host key that was approved during the preflight known-host check.

## Expected behavior summary

```text
preflight ssh_connect()
read host key
known-host decision / user approval
open real auth/shell connection
read host key again
compare key type + fingerprint
only then authenticate
```

If the second connection reports a different key:

```text
abort before authentication
show a host-key verification failure
state that authentication was not attempted
```

## Test A — normal saved xterm.js session

1. Start DD-SSH.
2. Double-click a trusted saved session.
3. Confirm terminal opens.
4. Run:

```bash
whoami
hostname
clear
```

Expected:

- terminal works as before
- no JSON/config format change
- no new known-host prompt for already trusted key

## Test B — saved auth test with correct credentials

1. Right-click saved session.
2. Run the existing connection/auth test.

Expected output includes:

```text
Host-key verification before auth: VERIFIED
Status: SUCCESS
```

## Test C — saved auth test with wrong password

1. Temporarily edit the saved password to a wrong value, or create a test session with a wrong password.
2. Run auth test.

Expected:

```text
Host-key verification before auth: VERIFIED
Status: FAILED
Authentication denied by server.
```

This confirms the host-key verification happens before auth, but auth failure still behaves normally.

## Test D — unknown host Trust once

1. Use a host/port not present in `known_hosts`, or temporarily move config aside.
2. Start terminal open.
3. Choose `Trust once`.

Expected:

- terminal opens for this attempt
- host key is not saved permanently
- auth/shell connection still verifies the same approved key before auth

## Test E — unknown host Trust permanently

1. Use a host/port not present in `known_hosts`.
2. Choose `Trust permanently`.
3. Open terminal.
4. Close and reconnect.

Expected:

- key is saved under `known_hosts`
- next connection is trusted
- terminal opens normally

## Test F — deliberate mismatch / host-key changed

Use a copied test config, not your real production config.

1. Close DD-SSH.
2. Backup config.
3. Edit the relevant `known_hosts` fingerprint for the same key type to a fake value.
4. Start DD-SSH.
5. Try to open/auth-test that session.

Expected:

- DD-SSH blocks with host-key changed / verification failure
- auth is not attempted
- no password/private key is sent

## Test G — multi-key portability regression

Fixture:

```text
host: 138.2.166.222
port: 223
ssh-ed25519: SHA256:b2bVKCQSkPXuvXn4blGPV91iuJ5ySA8PqrBsI/8i5hs
ecdsa-sha2-nistp256: SHA256:tXwRSs3yDB71wdVX8Cnj57dmCszCgtU1kIHnDS9i19w
```

Expected:

- ED25519 path works where negotiated
- ECDSA path works where negotiated
- additional-key trust flow is not broken

## Test H — Windows KEX regression

Fixture:

```text
lab.dd-lab.hr:2231
```

Expected on Windows 10/11:

- DD-SSH reaches auth/shell
- Windows libssh KEX workaround still works
- no `Failed to construct client init buffer`

## Pass criteria

This checkpoint passes when:

```text
Linux build works
trusted xterm session opens
manual/saved auth test shows host-key verification before auth
wrong password fails normally after verified host key
known-host changed blocks before auth
Windows standalone still works after pull/build/deploy
```
