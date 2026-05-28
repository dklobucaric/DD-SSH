# DD-SSH Config Management

DD-SSH is built around one primary file:

```text
dd-ssh.json
```

This file is intended to be portable, human-readable, and easy to back up.

## Default paths

Linux:

```text
~/.config/DD-LAB/DD-SSH/dd-ssh.json
```

Windows/macOS:

```text
Qt QStandardPaths::AppConfigLocation
```

The exact path is shown in:

```text
Help → About DD-SSH
Tools → Settings
```

## Config folder

Open it from:

```text
File → Open Config Folder
```

## Backups

When enabled in Settings, DD-SSH creates backups before saving.

Backup names look like:

```text
dd-ssh.json.bak-20260517-204435-715
```

Settings:

```text
Tools → Settings → Config safety
```

Options:

```text
Enable config backups before save
Keep last N backups
```

## Export

```text
File → Export Config...
```

Exports the active `dd-ssh.json` to a user-selected path.

Warning: exported configs may contain plaintext passwords and private keys.

## Import

```text
File → Import Config...
```

Import flow:

```text
1. User selects JSON file.
2. DD-SSH validates that it is valid JSON.
3. DD-SSH validates that the root is a JSON object.
4. DD-SSH warns that current sessions/secrets/known_hosts/settings will be replaced.
5. DD-SSH creates a pre-import backup of the active config.
6. DD-SSH copies the imported file as active dd-ssh.json.
7. DD-SSH reloads settings and session list.
```

## Restore latest backup

```text
File → Restore Latest Backup...
```

Restore flow:

```text
1. DD-SSH finds newest valid dd-ssh.json.bak-* file.
2. DD-SSH asks for confirmation.
3. Current active config is moved aside as dd-ssh.json.pre-restore-*.
4. Backup is copied back as dd-ssh.json.
5. Settings and session list reload.
```

## Corrupt config recovery

If active config is invalid, DD-SSH does not overwrite it.

The recovery dialog offers:

```text
Open config folder
Continue read-only
Restore latest valid backup
Create fresh config
```

### Continue read-only

Starts the app with default in-memory settings and an empty session list if needed. Save operations must not silently overwrite the corrupt file.

### Restore latest valid backup

Moves corrupt config aside and restores the latest valid backup.

### Create fresh config

Moves corrupt config aside and creates a new empty default `dd-ssh.json`.

## Manual safety commands

Before risky tests:

```bash
cp ~/.config/DD-LAB/DD-SSH/dd-ssh.json ~/.config/DD-LAB/DD-SSH/dd-ssh.json.manual-good
chmod 600 ~/.config/DD-LAB/DD-SSH/dd-ssh.json.manual-good
```

Validate JSON:

```bash
jq . ~/.config/DD-LAB/DD-SSH/dd-ssh.json >/dev/null && echo "JSON OK"
```

List backups:

```bash
ls -la ~/.config/DD-LAB/DD-SSH/dd-ssh.json*
```

## Import/export safety preview

Starting with `dev 0.1.6.8`, DD-SSH previews configs before import/export. The preview is informational and does not change the human-readable JSON schema. It shows session count, known-host count, trusted key count, secrets mode, saved secret count, plaintext-secret presence, settings presence, metadata presence, and warnings.

This is intended to make the current `plain-v1` portability model safer without introducing encryption or an opaque config format yet.
