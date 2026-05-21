# DD-SSH Linux Packaging

**Checkpoint:** dev 0.1.6.1 — Andromeda  
**Phase:** first Debian package experiment

This document describes the first Linux packaging path for DD-SSH.

The goal is not yet a perfect distribution-grade package. The goal is a practical first `.deb` that installs the tested DD-SSH binary, desktop launcher, icons, and documentation on Debian/Ubuntu/Mint-style systems.

## Packaging strategy

`dev 0.1.6.1` uses a **system-runtime Debian package**:

- DD-SSH is built locally with CMake.
- The package installs `/usr/bin/dd-ssh`.
- The package installs a desktop launcher under `/usr/share/applications`.
- PNG icons are installed into the hicolor icon theme.
- Markdown docs are installed under `/usr/share/doc/dd-ssh`.
- Qt/libssh runtime libraries are expected from the Linux distribution packages.

This keeps the first `.deb` small and easy to inspect. A later packaging checkpoint may add an AppDir/AppImage or a bundled runtime strategy if needed.

## Required packaging tools

On Debian/Ubuntu/Mint/LMDE-style systems, install the normal build dependencies first. For packaging, these tools are also useful:

```bash
sudo apt install dpkg-dev fakeroot desktop-file-utils hicolor-icon-theme
```

`dpkg-dev` provides `dpkg-deb` and `dpkg-shlibdeps`. The packaging script uses `dpkg-shlibdeps` when available to detect runtime library dependencies from the built binary.

## Build a Release binary

From the project root:

```bash
./scripts/linux-build-release.sh
```

The default output is:

```text
build-linux-release/dd-ssh
```

You can test it directly:

```bash
./build-linux-release/dd-ssh
```

## Create the first `.deb`

From the project root:

```bash
./scripts/linux-package-deb.sh
```

Expected output:

```text
dist/deb/dd-ssh_0.1.6.1_amd64.deb
```

Inspect the package metadata:

```bash
dpkg-deb -I dist/deb/dd-ssh_0.1.6.1_amd64.deb
```

List package contents:

```bash
dpkg-deb -c dist/deb/dd-ssh_0.1.6.1_amd64.deb
```

## Install locally

```bash
sudo apt install ./dist/deb/dd-ssh_0.1.6.1_amd64.deb
```

Then run:

```bash
dd-ssh
```

The desktop menu should also show DD-SSH under a network/remote-access category after the desktop database refreshes.

## Remove

```bash
sudo apt remove dd-ssh
```

This removes the installed application files. It does **not** delete the user config file. Per-user config remains in the normal Qt standard path, such as:

```text
~/.config/DD-Lab/DD-SSH/dd-ssh.json
```

## Important security note

Do not package a real `dd-ssh.json`. Early DD-SSH builds may store passwords/private keys in plaintext under `secrets.mode = plain-v1`. The `.deb` installs only application files and documentation.

## Dependency notes

The package script tries to detect shared-library dependencies automatically with `dpkg-shlibdeps`. If that fails, it falls back to a conservative dependency list for Qt 6, Qt WebEngine, and libssh.

If the fallback list does not match a specific distribution, override it manually:

```bash
DD_SSH_DEB_DEPENDS="libc6, libstdc++6, libgcc-s1, libssh-4, libqt6core6, libqt6gui6, libqt6widgets6, libqt6webchannel6, libqt6webenginewidgets6" ./scripts/linux-package-deb.sh
```

## Test checklist

After installing the `.deb`, verify:

```text
[ ] dd-ssh starts from terminal
[ ] desktop launcher appears
[ ] app icon appears
[ ] About shows dev 0.1.6.1
[ ] settings dialog opens
[ ] existing user config is preserved
[ ] password SSH login works
[ ] private-key SSH login works
[ ] xterm.js terminal opens
[ ] whoami works
[ ] htop works
[ ] exit safety still works with an active SSH session
```

## Current limitations

This first `.deb` is an experiment, not a final distribution policy package. It does not yet provide:

- AppImage
- Snap/Flatpak
- automatic update mechanism
- signed packages
- official repository/PPA
- bundled Qt runtime

Those can be added in later 0.1.6.x packaging checkpoints.
