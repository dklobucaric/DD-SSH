# DD-SSH Release Process

This process is still evolving.

## Current next target

Potential next public tag:

```text
v0.2.0-alpha — Andromeda
```

Meaning:

```text
Real Terminal Foundation public alpha
```

## Pre-release checklist

Before an alpha release:

- Version string updated in `CMakeLists.txt`
- About dialog shows correct version/codename/milestone
- README reflects current status
- SECURITY_NOTES warns about plaintext secrets
- TEST_MATRIX is current
- PUBLIC_ALPHA_CHECKLIST is completed
- RELEASE_NOTES_v0.2.0-alpha.md is reviewed
- GitHub issue templates are present
- CHANGELOG includes latest checkpoint
- Build works locally
- Password session test passes
- Private-key session test passes
- xterm terminal test passes
- PTY resize test passes
- htop/nano/vim/top/clear tests pass
- Remote reboot/reconnect test passes
- Config export/import/recovery tests pass

## Suggested release note structure

```text
DD-SSH v0.2.0-alpha — Andromeda

This is a public alpha for Real Terminal Foundation testing.

Highlights:
- Saved SSH sessions
- xterm.js terminal tabs
- Password/private-key auth
- known_hosts handling
- PTY resize
- Config import/export/recovery

Security warning:
- Saved passwords/private keys are plaintext in dd-ssh.json.

Known limitations:
- Linux tested most heavily
- No encrypted secrets yet
- No SFTP
- No Multi-Exec yet
```


## Release artifacts

Do not commit generated packages or deployment folders. Upload them to GitHub Releases.

Expected release assets may include:

```text
DD-SSH-dev-0.1.6.5-windows-portable.zip
dd-ssh_0.1.6.5_amd64.deb
DD-SSH-0.1.6.5-macOS-x86_64.dmg
SHA256SUMS
```

Generate checksums before uploading:

```bash
./scripts/generate-checksums-linux.sh
./scripts/generate-checksums-macos.sh
```

Windows:

```cmd
scripts\generate-checksums-windows.bat
```

See [Release Artifacts](RELEASE_ARTIFACTS.md).

## Tagging

Example:

```bash
git tag -a v0.2.0-alpha -m "DD-SSH v0.2.0-alpha Andromeda"
git push origin v0.2.0-alpha
```

Do not tag until tests are confirmed.


---

## Windows Release build gate

Before a public alpha tag that claims Windows support, perform the Windows Release build process from [Windows Build Guide](WINDOWS_BUILD.md).

Record:

```text
- Windows version
- Qt version
- vcpkg/libssh version
- Release configure/build result
- app startup time
- first terminal startup time
- second terminal startup time
- RAM after Welcome screen
- RAM after one xterm terminal
```

Deployment/installer artifacts are not required for the first internal Windows validation, but public releases should eventually use `windeployqt` and a clean machine/runtime test.


---

## Windows deployment folder test

Current macOS packaging checkpoint: `dev 0.1.6.5 — macOS DMG/dependency polish`.

Before a public alpha release that includes Windows notes:

```text
1. Build Release on Windows.
2. Run scripts\windows-deploy-release.bat.
3. Start dist\windows-release\dd-ssh.exe from a normal Command Prompt.
4. Confirm About, Settings, xterm terminal, SSH login, htop, Disconnect, and Reconnect.
```

See `docs/WINDOWS_DEPLOYMENT.md` for details.
