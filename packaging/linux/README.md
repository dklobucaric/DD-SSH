# DD-SSH Linux packaging assets

This directory contains early Linux packaging helpers.

The first Debian package experiment is intentionally simple:

- build from the local CMake Release tree
- install the binary, desktop launcher, icons, and documentation under `/usr`
- generate a `.deb` with `dpkg-deb`
- use distro Qt/libssh runtime packages instead of bundling Qt libraries

See `docs/LINUX_PACKAGING.md`.
