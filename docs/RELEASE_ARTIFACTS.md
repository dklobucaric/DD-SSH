# DD-SSH release artifacts

**Checkpoint:** dev 0.1.6.6 — Andromeda  
**Phase:** repo hygiene and release artifact workflow

This repository should contain source code, documentation, resources, packaging templates, and helper scripts. Generated release artifacts should not be committed to Git.

## Do not commit

The following are generated outputs and should stay out of the repository history:

```text
build/
build-*/
build-linux-release/
build-win-release/
build-macos-release/
dist/
*.deb
*.dmg
*.zip
*.AppImage
*.msi
*.pkg
*.tar.gz
*.tgz
.DS_Store
__MACOSX/
Thumbs.db
desktop.ini
```

The `.gitignore` file added in this checkpoint protects these paths and file types.

## Where artifacts belong

Release artifacts belong in GitHub Releases, not in normal Git commits.

Typical future release assets:

```text
DD-SSH-dev-0.1.6.5-windows-portable.zip
dd-ssh_0.1.6.5_amd64.deb
DD-SSH-0.1.6.5-macOS-x86_64.dmg
SHA256SUMS
```

## Checksum helpers

DD-SSH includes platform helpers for generating SHA256 checksums from release artifacts under `dist/`.

### Linux

```bash
./scripts/generate-checksums-linux.sh
```

### macOS

```bash
./scripts/generate-checksums-macos.sh
```

### Windows

From Command Prompt:

```cmd
scripts\generate-checksums-windows.bat
```

Or directly with PowerShell:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\generate-checksums-windows.ps1
```

All helpers write:

```text
dist/SHA256SUMS
```

## Supported artifact extensions

The checksum helpers intentionally scan only release-package style files:

```text
.zip
.deb
.dmg
.AppImage
.msi
.pkg
.tar.gz
.tgz
```

They do not checksum every file inside a Windows portable folder. Create a portable `.zip` first, then generate checksums for that `.zip`.

## Example release flow

```bash
# Linux .deb
./scripts/linux-build-release.sh
DD_SSH_DEB_VERSION=0.1.6.5 ./scripts/linux-package-deb.sh

# Optional: put/copy all final assets under dist/ before generating checksums
./scripts/generate-checksums-linux.sh
cat dist/SHA256SUMS
```

Windows portable release example:

```text
1. Build Release on Windows.
2. Run scripts\windows-deploy-release.bat.
3. Zip dist\windows-release as DD-SSH-dev-0.1.6.5-windows-portable.zip.
4. Place/copy the zip under dist\.
5. Run scripts\generate-checksums-windows.bat.
6. Upload the zip and dist\SHA256SUMS to GitHub Releases.
```

macOS release example:

```bash
./scripts/macos-build-release.sh
DD_SSH_MACOS_VERSION=0.1.6.5 ./scripts/macos-deploy-release.sh
./scripts/generate-checksums-macos.sh
```

## Source ZIP note

A source-code ZIP prepared for testing is not a repository artifact. It may be shared manually for review, but it should not be committed back into the repository.
