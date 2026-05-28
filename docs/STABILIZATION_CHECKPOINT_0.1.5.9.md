# DD-SSH dev 0.1.5.9 Stabilization Checkpoint

**Checkpoint:** dev 0.1.5.9 — Andromeda
**Focus:** documentation, validation summary, and public-alpha release polish
**Milestone:** MF 0.2 candidate — Real Terminal Foundation

This checkpoint intentionally does **not** add new SSH/session/terminal runtime features. It records the successful stabilization work completed across the previous three checkpoints and aligns the documentation with the current tested state.

## What this checkpoint locks in

### dev 0.1.5.6 — Windows standalone deployment

Validated result:

```text
Windows Release build works.
The dist\windows-release folder can run outside the build tree.
The app launches on real Windows 10 and Windows 11 machines.
Qt/vcpkg developer PATH is not required for the deployed folder test.
```

Confirmed during testing:

- app launch
- app icon
- About dialog
- Settings dialog
- AppData config path
- Linux config import
- password SSH login
- xterm.js terminal
- `whoami`
- `htop`
- exit safety when connected sessions exist

### dev 0.1.5.7 — Known-host multi-key portability

Validated result:

```text
One portable dd-ssh.json can contain multiple legitimate host-key algorithms
for the same host:port.
```

Real regression case:

```text
Host: 138.2.166.222
Port: 223

Linux / Windows 11 negotiated: ssh-ed25519
Windows 10 negotiated: ecdsa-sha2-nistp256
```

Confirmed legitimate server fingerprints:

```text
ssh-ed25519:
SHA256:b2bVKCQSkPXuvXn4blGPV91iuJ5ySA8PqrBsI/8i5hs

ecdsa-sha2-nistp256:
SHA256:tXwRSs3yDB71wdVX8Cnj57dmCszCgtU1kIHnDS9i19w
```

Expected post-fix behavior:

```text
Additional legitimate key type → Trust additional key → save both keys.
True same-algorithm fingerprint change → strong SSH host-key-changed warning.
```

### dev 0.1.5.8 — Windows libssh KEX compatibility

Validated result:

```text
Windows DD-SSH can connect to a modern OpenSSH server that previously failed
with: Failed to construct client init buffer.
```

Real regression case:

```text
Host: lab.dd-lab.hr
Port: 2231
Forwarded target: 192.168.1.233:223
Server banner: OpenSSH_10.0p2 Debian-7+deb13u2
```

Observed before the fix:

```text
Linux DD-SSH: works
Windows OpenSSH: works and reaches authentication
Windows DD-SSH dev build: fails during handshake
Windows DD-SSH standalone build: fails during handshake
```

A temporary server-side KEX restriction proved the root cause. After the app-side Windows KEX compatibility override, the server-side workaround is no longer required for the validated case.

Tested after the fix:

```text
Windows 10: PASS
Windows 11: PASS
Linux: PASS
```

## Current state summary

```text
Linux build/runtime: PASS
Windows 10 build/runtime/deploy-folder: PASS
Windows 11 build/runtime/deploy-folder: PASS
Portable config import Linux → Windows: PASS
Known-host multi-key portability: PASS
Windows libssh KEX compatibility: PASS
macOS validation: TODO
Installer/code signing: TODO
Encrypted secrets: TODO
```

## Suggested tag

If the repository is clean and the latest build still passes, this checkpoint is a good candidate for a development tag:

```bash
git tag dev-0.1.5.9
git push origin dev-0.1.5.9
```

This is not a public release tag. It is a stable internal development marker before the next public-alpha preparation pass.
