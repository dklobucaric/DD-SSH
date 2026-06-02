# Testcase — dev 0.1.8.8

## Settings dialog layout

- [ ] About shows `dev 0.1.8.8`.
- [ ] Tools → Settings opens without clipping on macOS.
- [ ] Left sidebar categories are visible: General, Diagnostics, Appearance, Terminal, Config safety, Security note.
- [ ] General page shows a readable config path field.
- [ ] Copy path copies the active `dd-ssh.json` path.
- [ ] Open folder opens the config folder.
- [ ] Double-click session row says Open terminal.
- [ ] Quick toolbar checkbox saves correctly.
- [ ] Diagnostics page checkbox toggles diagnostic logging after OK.
- [ ] Appearance page theme selector saves correctly.
- [ ] Terminal font family and font size save correctly and apply to newly opened terminal tabs.
- [ ] Config safety backup settings save correctly.
- [ ] Security note page clearly shows the plain-v1 warning.
- [ ] Cancel closes without saving changed settings.

## Regression smoke

- [ ] Existing terminal connection still works.
- [ ] Multiline paste in nano/YAML still preserves formatting.
- [ ] File Manager opens.
- [ ] Queue upload/download/delete still works.
- [ ] Diagnostic logging still writes logs only when enabled.
