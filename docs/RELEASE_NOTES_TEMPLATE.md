# DD-SSH release notes template

Version: `dev 0.1.x` or `v0.2.0-alpha`  
Codename: Andromeda  
Date: YYYY-MM-DD  
Type: internal checkpoint / public alpha / bugfix

## Highlights

- ...

## Fixed

- ...

## Changed

- ...

## Not changed

- SSH core unchanged unless explicitly listed.
- Human-readable `dd-ssh.json` format remains in place unless explicitly listed.

## Security notes

- Saved `plain-v1` secrets are human-readable plaintext in `dd-ssh.json`.
- Do not upload real configs, private keys, or passwords to bug reports.

## Downloads

- Windows portable ZIP: `...`
- Debian package: `...`
- macOS Intel DMG: `...`
- Checksums: `SHA256SUMS`

## SHA256 checksums

Attach generated `SHA256SUMS` to the GitHub Release.

## Tested platforms

| Platform | Result | Notes |
| --- | --- | --- |
| Linux Debian/Ubuntu/Mint style | | |
| Windows 10 | | |
| Windows 11 | | |
| macOS Intel | | |
| macOS Apple Silicon / Rosetta | | |

## Known issues

- ...

## Upgrade notes

- User config remains in the platform-specific Qt config directory.
- Back up `dd-ssh.json` before testing if it contains real sessions/secrets.
