# DD-SSH Troubleshooting

## Build problems

### Qt6WebEngineWidgetsConfig.cmake missing

Install Qt WebEngine development package:

```bash
sudo apt install qt6-webengine-dev
```

Then re-run CMake:

```bash
cmake -S . -B build -G Ninja
```

### libssh not found

Install libssh development package:

```bash
sudo apt install libssh-dev
```

### XKB warning during build

You may see a warning about XKB libraries. If the app builds and runs, it may be harmless.

To clean it up on Debian/Ubuntu/Mint:

```bash
sudo apt install libxkbcommon-dev libxkbcommon-x11-dev libxcb-xkb-dev
```

## Config problems

### Where is my config?

Open:

```text
Help → About DD-SSH
```

or:

```text
Tools → Settings
```

Linux default:

```text
~/.config/DD-LAB/DD-SSH/dd-ssh.json
```

### Config is corrupt

DD-SSH should show recovery dialog.

Options:

```text
Continue read-only
Restore latest valid backup
Create fresh config
Open config folder
```

Manual validation:

```bash
jq . ~/.config/DD-LAB/DD-SSH/dd-ssh.json >/dev/null && echo "JSON OK"
```

### Restore manually

```bash
cp ~/.config/DD-LAB/DD-SSH/dd-ssh.json.bak-YYYYMMDD-HHMMSS-XXX ~/.config/DD-LAB/DD-SSH/dd-ssh.json
chmod 600 ~/.config/DD-LAB/DD-SSH/dd-ssh.json
```

## SSH problems

### Wrong password

Use:

```text
Session → Connect / Auth test
```

Verify host, port, username, and password.

### New session was not saved

DD-SSH saves a new session only after successful SSH authentication.

If the host is unreachable, the port is wrong, credentials are wrong, the private key fails, or the known-host decision does not allow continuing, DD-SSH will not write that session to `dd-ssh.json`.

This is expected behavior. First make the connection test pass, then save the session.


### Private key auth fails

Check:

- Key file is valid
- Correct user
- Correct host/port
- Server has matching public key in `authorized_keys`
- Key content was saved correctly if using portable plaintext key storage

### Windows handshake fails with "Failed to construct client init buffer"

`dev 0.1.5.8+` includes a Windows-only libssh KEX compatibility override for newer OpenSSH servers that advertise ML-KEM/SNTRUP key-exchange algorithms before classic curve25519/ecdh algorithms.

If you need to compare behavior, run from CMD:

```cmd
set DD_SSH_DISABLE_WINDOWS_KEX_COMPAT=1
dd-ssh.exe
```

To enable libssh protocol verbosity while debugging:

```cmd
set DD_SSH_LIBSSH_DEBUG=1
dd-ssh.exe
```

Normal users should not need either variable.

### Known-host warning

If host is unknown, DD-SSH asks for trust confirmation.

If host key changed, do not blindly accept. The server may have been reinstalled, DNS/IP may have changed, or there may be a security issue.

## Terminal problems

### htop/nano/vim looks broken

Check header says:

```text
xterm.js ACTIVE - local bundled renderer
```

If fallback is active, local xterm resources are not loading.

### stty size does not change after resize

Run:

```bash
stty size
```

Resize DD-SSH window and run again. If it does not change, PTY resize sync may be broken.

### Terminal disconnected after reboot

This is expected when the remote host reboots.

Click:

```text
Reconnect
```

when the server comes back.

## Import/export problems

### Import rejected

DD-SSH requires imported config to be valid JSON and a JSON object.

Validate:

```bash
jq . imported-file.json
```

### Exported config contains secrets

Yes. Export copies full `dd-ssh.json`, including plaintext secrets.

Do not share exported configs publicly.
