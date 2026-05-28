# DD-SSH dev 0.1.6.3 — Build and test notes

Checkpoint: `dev 0.1.6.3`
Focus: SSH trust-chain hardening before authentication

## Scope

This checkpoint is intentionally small.

Changed:

- `src/ssh/SshSession.h/.cpp`
- `src/ssh/SshShellWorker.h/.cpp`
- `src/ui/WebTerminalTab.h/.cpp`
- `src/ui/BasicTerminalTab.h/.cpp`
- `src/ui/MainWindow.cpp`
- `CMakeLists.txt`
- README/security/changelog/test documentation

Not changed:

- Human-readable `dd-ssh.json` format
- `plain-v1` secrets model
- Config import/export format
- Known-host JSON storage schema
- Windows KEX compatibility override
- Packaging scripts
- xterm.js terminal transport model
- File manager / SFTP / traffic widget roadmap items

## Linux build

From a clean source folder:

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

Expected About dialog version:

```text
dev 0.1.6.3
Andromeda
MF 0.2 candidate
```

## Linux smoke test

1. Open an existing trusted saved session with xterm.js.
2. Confirm the terminal opens normally.
3. Run:

```bash
whoami
hostname
clear
stty size
```

4. Run one fullscreen terminal app if available:

```bash
htop
```

5. Disconnect and reconnect once.

## Authentication test smoke test

Run a saved-session or manual auth test against a known good host.

Expected output should include:

```text
Authentication result:
Host-key verification before auth: VERIFIED
Auth connection key type: ...
Auth connection fingerprint: ...
Status: SUCCESS
```

For a wrong password test, the host-key verification should still show `VERIFIED`, then authentication should fail normally.

## Regression tests that matter most

### 1. Trusted host terminal open

Expected:

- known-host decision is trusted
- terminal opens
- password/private key auth still works

### 2. Unknown host trust once

Expected:

- unknown host prompt appears
- Trust once continues only this attempt
- terminal opens only after the worker verifies the same approved key before auth

### 3. Unknown host trust permanently

Expected:

- host key is saved to `dd-ssh.json`
- terminal opens
- next connection is trusted

### 4. Same key type, different fingerprint

Expected:

- strong host-key changed warning
- authentication is not attempted

### 5. Additional key type portability

Known regression fixture:

```text
host: 138.2.166.222
port: 223
ssh-ed25519: SHA256:b2bVKCQSkPXuvXn4blGPV91iuJ5ySA8PqrBsI/8i5hs
ecdsa-sha2-nistp256: SHA256:tXwRSs3yDB71wdVX8Cnj57dmCszCgtU1kIHnDS9i19w
```

Expected:

- Linux/Windows 11 ED25519 path still works
- Windows 10 ECDSA path still works
- one `host:port` can still carry both keys

### 6. Windows OpenSSH 10 KEX compatibility

Known regression host:

```text
lab.dd-lab.hr:2231
```

Expected on Windows:

- Windows-safe KEX override still reaches auth/shell
- no return of `Failed to construct client init buffer`

## Windows workflow after Linux validation

After Linux passes:

```bash
git status
git add .
git commit -m "Harden SSH host-key verification before auth for dev 0.1.6.3"
git push origin main
```

Then on Windows:

```text
git pull
run native Windows build
run scripts\windows-deploy-release.bat
launch dist\windows-release\dd-ssh.exe
repeat trusted host / wrong password / xterm smoke tests
```

## Notes from ChatGPT build environment

CMake configure could not be completed in the sandbox because Qt6 development packages are not installed there:

```text
Could not find a package configuration file provided by "Qt6"
```

So this checkpoint still requires your normal Linux Qt build machine for the real compile/link test.
