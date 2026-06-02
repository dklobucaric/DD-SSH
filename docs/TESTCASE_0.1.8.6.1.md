# DD-SSH dev 0.1.8.6.1 — Test case

This checkpoint fixes release artifact naming only. Runtime behavior should remain unchanged from `dev 0.1.8.6`.

## Checks

- [ ] About shows `dev 0.1.8.6.1`.
- [ ] `scripts/macos-deploy-release.sh` no longer defaults to `0.1.7.1`.
- [ ] macOS DMG filename matches the current checkpoint version by default.
- [ ] macOS otool report filename and internal report Version match the current checkpoint version by default.
- [ ] `DD_SSH_MACOS_VERSION=...` still overrides the generated artifact version.
- [ ] `scripts/linux-package-deb.sh` no longer defaults to `0.1.7.1`.
- [ ] `DD_SSH_DEB_VERSION=...` still overrides the Debian package version.
- [ ] File Manager, terminal, upload/download queue, local/remote delete, and diagnostic logging still smoke-test successfully.
