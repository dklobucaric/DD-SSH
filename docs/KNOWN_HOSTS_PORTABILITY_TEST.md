# DD-SSH Known-host Portability Regression Test

**Checkpoint:** dev 0.1.5.7 — Andromeda  
**Focus:** multi-key known-host storage for one portable `dd-ssh.json`

This test documents the real cross-platform issue found during the Windows standalone deployment test.

## Test server

```text
Host: 138.2.166.222
Port: 223
```

Confirmed server-side SSH host-key fingerprints:

```text
ssh-ed25519:
SHA256:b2bVKCQSkPXuvXn4blGPV91iuJ5ySA8PqrBsI/8i5hs

ecdsa-sha2-nistp256:
SHA256:tXwRSs3yDB71wdVX8Cnj57dmCszCgtU1kIHnDS9i19w
```

## Why this matters

The same SSH server can legitimately expose multiple host-key algorithms. Different OS/libssh combinations can negotiate different algorithms.

Observed behavior before dev 0.1.5.7:

```text
Linux / Windows 11 negotiate ssh-ed25519
Windows 10 negotiates ecdsa-sha2-nistp256
```

The old DD-SSH known-host model stored only one key per `host:port`, so replacing the stored key on one platform broke the same portable JSON on another platform.

## Expected config model after 0.1.5.7

```json
"known_hosts": {
  "138.2.166.222:223": {
    "keys": {
      "ssh-ed25519": "SHA256:b2bVKCQSkPXuvXn4blGPV91iuJ5ySA8PqrBsI/8i5hs",
      "ecdsa-sha2-nistp256": "SHA256:tXwRSs3yDB71wdVX8Cnj57dmCszCgtU1kIHnDS9i19w"
    },
    "first_seen": "...",
    "last_seen": "..."
  }
}
```

## Regression fixture A — ED25519-only config

Starting condition:

```text
Known-host entry contains only ssh-ed25519.
This works on Linux / Windows 11.
This used to fail on Windows 10 when libssh negotiated ECDSA.
```

Expected dev 0.1.5.7 behavior on Windows 10:

1. DD-SSH detects the host is known but the current key type is not saved yet.
2. Dialog title: `Additional SSH host key`.
3. Available choices include `Trust additional key`, `Trust once`, and `Cancel`.
4. Click `Trust additional key`.
5. Connection continues.
6. JSON now stores both `ssh-ed25519` and `ecdsa-sha2-nistp256` for the same `host:port`.
7. Copy the same JSON back to Linux / Windows 11.
8. Linux / Windows 11 connects without a reverse host-key warning.

## Regression fixture B — ECDSA-only config

Starting condition:

```text
Known-host entry contains only ecdsa-sha2-nistp256.
This works on Windows 10.
This used to fail on Linux / Windows 11 when libssh negotiated ED25519.
```

Expected dev 0.1.5.7 behavior on Linux / Windows 11:

1. DD-SSH detects the host is known but the current key type is not saved yet.
2. Dialog title: `Additional SSH host key`.
3. Click `Trust additional key`.
4. Connection continues.
5. JSON now stores both keys.
6. Copy the same JSON back to Windows 10.
7. Windows 10 connects without a reverse host-key warning.

## Trust once test

Starting condition:

```text
Known-host entry contains one legitimate key.
Current platform negotiates the other legitimate key.
```

Expected behavior:

1. At the `Additional SSH host key` prompt, click `Trust once`.
2. The current auth/shell attempt continues.
3. `dd-ssh.json` is not permanently changed.
4. Restart DD-SSH and retry the same connection.
5. The additional-key prompt appears again.

## True host-key-changed test

Use only a copied test JSON.

1. Edit one stored fingerprint for an existing key type.
2. Keep the key type the same.
3. Start DD-SSH and connect.
4. Expected result: strong `SSH host key changed` warning.
5. DD-SSH must not treat this as an additional-key case.

