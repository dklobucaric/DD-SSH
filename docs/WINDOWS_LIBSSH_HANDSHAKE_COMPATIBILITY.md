# DD-SSH Windows libssh Handshake Compatibility Test

**Checkpoint:** dev 0.1.5.9 — Andromeda
**Focus:** Windows libssh KEX compatibility with newer OpenSSH servers

## Real regression case

Problematic connection:

```text
Host: lab.dd-lab.hr
Port: 2231
Forwarded target: 192.168.1.233:223
Server banner: OpenSSH_10.0p2 Debian-7+deb13u2
```

Observed before dev 0.1.5.8:

```text
Linux DD-SSH: works
Windows OpenSSH: works and reaches authentication
Windows DD-SSH dev build: fails during handshake
Windows DD-SSH standalone build: fails during handshake
DD-SSH error: Failed to construct client init buffer
```

The same Windows failure occurred through the public port forward, through VPN, and through the local network path. This ruled out DNS and port forwarding as the primary cause.

## Why this happens

The server advertises modern OpenSSH key-exchange algorithms first, including:

```text
mlkem768x25519-sha256
sntrup761x25519-sha512
sntrup761x25519-sha512@openssh.com
curve25519-sha256
curve25519-sha256@libssh.org
```

Windows OpenSSH successfully negotiated:

```text
kex: algorithm: curve25519-sha256
kex: host key algorithm: ssh-ed25519
```

Windows libssh/vcpkg failed before authentication with:

```text
Failed to construct client init buffer
```

## dev 0.1.5.8+ behavior

On Windows builds only, DD-SSH sets a conservative KEX list before `ssh_connect()`:

```text
curve25519-sha256,
curve25519-sha256@libssh.org,
ecdh-sha2-nistp256,
ecdh-sha2-nistp384,
ecdh-sha2-nistp521,
diffie-hellman-group14-sha256
```

This applies to:

- Manual handshake tests
- Authentication tests
- Real shell sessions

Linux and future macOS builds do not apply this Windows-only override.


## Validation result

Current validated result as of dev 0.1.5.9 docs:

```text
Windows 10 DD-SSH: PASS
Windows 11 DD-SSH: PASS
Linux DD-SSH: PASS
Server-side KEX workaround: no longer required for this DD-SSH regression case
```

## Diagnostic switches

Enable libssh protocol verbosity locally:

```cmd
set DD_SSH_LIBSSH_DEBUG=1
dist\windows-release\dd-ssh.exe
```

Disable the Windows KEX compatibility override for comparison testing:

```cmd
set DD_SSH_DISABLE_WINDOWS_KEX_COMPAT=1
dist\windows-release\dd-ssh.exe
```

Do not ship normal user runs with these variables set unless debugging.

## Expected test

1. Build DD-SSH dev 0.1.5.9 on Windows.
2. Deploy with `scripts\windows-deploy-release.bat`.
3. Open the problematic saved session to `lab.dd-lab.hr:2231`.
4. Expected result: handshake succeeds and DD-SSH reaches known-host/auth/shell flow.
5. Optional comparison: set `DD_SSH_DISABLE_WINDOWS_KEX_COMPAT=1`; the old failure may return on affected Windows/libssh builds.

## Server-side workaround used to prove the bug

This temporary server config made Windows DD-SSH work before the app-side fix:

```text
KexAlgorithms curve25519-sha256,curve25519-sha256@libssh.org,ecdh-sha2-nistp256,ecdh-sha2-nistp384,ecdh-sha2-nistp521,diffie-hellman-group14-sha256
```

Validated after dev 0.1.5.8 on two Windows machines and one Linux machine: the server-side compatibility file is no longer needed for DD-SSH in this regression case.
