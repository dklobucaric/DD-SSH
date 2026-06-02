# Test case — dev 0.1.8.9 SFTP traffic monitor integration

## Version

- [ ] `Help → About` shows `dev 0.1.8.9`

## Terminal regression

- [ ] Open a normal terminal tab
- [ ] Run commands that produce output
- [ ] Session Traffic still updates for terminal SSH traffic
- [ ] Ctrl+C, paste, and multiline nano/YAML paste still work

## SFTP traffic smoke

- [ ] Open File Manager for a saved session
- [ ] Select the File Manager tab
- [ ] Status bar shows the File Manager/SFTP traffic context
- [ ] Upload a file large enough to observe traffic, for example 10–100 MB
- [ ] Up/sent rate and total increase during upload
- [ ] Download a file large enough to observe traffic
- [ ] Down/received rate and total increase during download
- [ ] Queue multiple upload/download files and start the queue
- [ ] Traffic totals continue increasing during queue items
- [ ] Queue a small folder upload/download and confirm folder-expanded file items update traffic totals

## Operations not counted as bulk traffic

- [ ] Refresh remote directory; traffic widget should not show a large transfer total change
- [ ] Create directory queue item; no large transfer total expected
- [ ] Delete local/remote file; no large transfer total expected

## Logging/privacy regression

- [ ] Diagnostic logging remains OFF by default
- [ ] Enabling diagnostic logging still logs useful file-transfer events
- [ ] Logs do not contain passwords, private keys, secret values, file contents, terminal input/output, or clipboard contents

## Cross-platform smoke

- [ ] Linux: traffic widget updates during SFTP upload/download
- [ ] Windows: traffic widget updates during SFTP upload/download
- [ ] macOS: traffic widget updates during SFTP upload/download
