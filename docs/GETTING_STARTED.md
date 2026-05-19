# Getting Started with DD-SSH

This guide explains how to build DD-SSH, create the first saved session, and open a terminal.

## 1. Build DD-SSH

On Debian/Ubuntu/Linux Mint style systems:

```bash
sudo apt update
sudo apt install -y \
  build-essential \
  cmake \
  ninja-build \
  git \
  pkg-config \
  qt6-base-dev \
  qt6-base-dev-tools \
  qt6-tools-dev \
  qt6-tools-dev-tools \
  qt6-webengine-dev \
  libssh-dev

cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

If CMake cannot find Qt WebEngine, install `qt6-webengine-dev`.

## Windows build note

A native Windows build path is documented separately because it requires MSVC, Qt MSVC, vcpkg `libssh`, and vcpkg `pkgconf`.

See: [Windows Build Guide](WINDOWS_BUILD.md).

## 2. Confirm version

Open:

```text
Help → About DD-SSH
```

Expected checkpoint:

```text
Version: dev 0.1.5.3
Codename: Andromeda
Milestone: MF 0.2 candidate
```

The About dialog also shows the active config path.

## 3. Create the first saved session

Open:

```text
Session → New Session
```

Enter:

```text
Host
Port
Username
Auth type
Password or private key
Session name
Group (optional)
```

`New Session` is intended to create a saved session after successful authentication.

The saved session appears in the left sidebar.

## 4. Open the terminal

Double-click the saved session in the left sidebar.

Expected result:

```text
xterm.js terminal opens
SSH authenticates using saved secret
Remote shell appears
```

Try:

```bash
whoami
hostname
pwd
stty size
htop
nano /tmp/dd-ssh-test.txt
vim /tmp/dd-ssh-test.txt
clear
exit
```

## 5. Manual connect/auth test

Open:

```text
Session → Connect / Auth test
```

This is a manual connection test. Saving is optional.

The same action is available from the sidebar context menu as `Run auth test`.

## 6. Edit/delete sessions

Right-click a saved session in the sidebar:

```text
Open xterm.js terminal
Run auth test
Open basic shell fallback
Edit session
Delete session
```

Session editing preserves the existing secret if password/key fields are left empty.

Deleting a session does not delete `known_hosts` records automatically.

## 7. Settings

Open:

```text
Tools → Settings
```

Current settings include:

- Config path display
- App theme: System / Light / Dark
- Terminal font family and size for new tabs
- Config backup policy
- Optional quick action toolbar
- Plaintext secrets warning

## 8. Config export/import

Open:

```text
File → Export Config...
File → Import Config...
File → Restore Latest Backup...
File → Open Config Folder
```

Config import/export operates on the full `dd-ssh.json`, not only settings.

That means exported configs may include plaintext passwords and private keys.

## 9. If config is corrupt

If `dd-ssh.json` is invalid, DD-SSH will not overwrite it automatically.

The recovery dialog offers:

```text
Open config folder
Continue read-only
Restore latest valid backup
Create fresh config
```

Corrupt configs are moved aside as:

```text
dd-ssh.json.corrupt-<timestamp>
```

when a recovery action creates or restores a config.
