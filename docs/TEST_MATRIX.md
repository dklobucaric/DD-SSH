# DD-SSH Test Matrix

**Checkpoint:** dev 0.1.7.5 — Andromeda  
**Milestone:** Local + remote read-only file manager foundation  
**Phase:** First two-panel read-only file manager, no transfers yet

## dev 0.1.7.5 — local + remote read-only file manager smoke test

[ ] About shows dev 0.1.7.5  
[ ] Saved-session context menu shows `Open File Manager (read-only two-panel)`  
[ ] File Manager opens with a local panel on the left and remote SFTP panel on the right  
[ ] Local panel starts at the user's home directory or a valid local path  
[ ] Local `Go`, `↑ Up`, `Refresh`, and double-click folder navigation work  
[ ] Local file selection does not upload/open/modify anything  
[ ] Remote panel still lists remote `.` on a known SFTP-enabled server  
[ ] Remote `Go`, `↑ Up`, `Refresh`, and double-click folder navigation still work  
[ ] No upload/download/delete/rename buttons are present  
[ ] Exit safety still lists open File Manager/SFTP tabs  
[ ] Existing xterm.js terminal, paste paths, Ctrl+C interrupt, known-host checks, and logging still work  
[ ] Session Traffic still reports terminal traffic only; SFTP transfer counters are not expected yet  

## dev 0.1.7.4.1 — read-only SFTP browser bugfix smoke test

[ ] About shows dev 0.1.7.4.1  
[ ] Saved-session context menu shows `Open File Manager (read-only)`  
[ ] Browser opens and lists remote `.` on a known SFTP-enabled server  
[ ] SFTP file table has readable rows in dark theme; no white alternating rows with white text  
[ ] Browser button shows `↑ Up` and navigates to the parent path  
[ ] With many tabs open, tab bar still allows navigation/scrolling where the platform supports it  
[ ] Closing the whole app with active SSH terminal tab(s) lists SSH terminal sessions in the confirmation  
[ ] Closing the whole app with open SFTP browser tab(s) lists SFTP browser tabs in the confirmation  
[ ] Closing the whole app with both SSH and SFTP tabs lists both groups  
[ ] Cancelling the exit confirmation keeps the app open  
[ ] SFTP traffic is not expected in the live Session Traffic monitor yet  
[ ] Existing xterm.js terminal, paste paths, Ctrl+C interrupt, known-host checks, and logging still work  

## dev 0.1.7.4 — read-only remote file browser smoke test

[ ] About shows dev 0.1.7.4  
[ ] Welcome tab mentions the read-only remote file browser  
[ ] Saved-session context menu shows `Open File Manager (read-only)`  
[ ] Browser uses the existing known-host prompt for unknown/additional/changed hosts  
[ ] Browser verifies the approved host key before authentication  
[ ] Password-auth saved session can open the browser and list remote `.`  
[ ] Private-key saved session can open the browser and list remote `.`  
[ ] Browser table shows Name / Type / Size / Modified / Permissions  
[ ] Double-clicking a directory navigates into it  
[ ] `Up` navigates to the parent directory  
[ ] Manual path entry + `Go` works for a known path  
[ ] `Refresh` reloads the current directory  
[ ] Double-clicking a regular file does not upload/download/open/modify it  
[ ] Existing saved xterm.js terminal still opens normally after using the browser  
[ ] Existing paste paths still work: toolbar Paste, right-click paste, Ctrl+Shift+V, Command+V  
[ ] Existing Ctrl+C remote interrupt still works  
[ ] Session Traffic still updates for terminal tabs  
[ ] Diagnostic logging still stays OFF by default  
[ ] Diagnostic logging, if enabled, logs SFTP summary events only and does not log file contents or secrets

## dev 0.1.7.3 — SFTP connection proof-of-concept smoke test

[ ] About shows dev 0.1.7.3  
[ ] Welcome tab mentions the SFTP connection proof of concept  
[ ] Saved-session context menu shows `Open File Manager (SFTP probe)`  
[ ] SFTP probe uses the existing known-host prompt for unknown/additional/changed hosts  
[ ] SFTP probe verifies the approved host key before authentication  
[ ] Password-auth saved session can initialize SFTP and list remote `.`  
[ ] Private-key saved session can initialize SFTP and list remote `.`  
[ ] SFTP subsystem failure is reported cleanly when the server rejects SFTP  
[ ] Secrets and file contents are not logged  
[ ] Existing saved xterm.js terminal still opens normally after the SFTP probe

## dev 0.1.7.2 — file transport architecture foundation smoke test

[ ] About shows dev 0.1.7.2  
[ ] Welcome tab mentions the File Transfer / File Manager architecture foundation  
[ ] Saved-session context menu shows `Open File Manager (planned)`  
[ ] Clicking `Open File Manager (planned)` shows an information dialog only  
[ ] Clicking the File Manager placeholder does not open an SSH/SFTP connection  
[ ] Clicking the File Manager placeholder does not change `dd-ssh.json`  
[ ] Existing saved xterm.js terminal still opens normally

## dev 0.1.7.1 — native paste event hardening

[PASS] Linux toolbar Paste  
[PASS] Linux right-click paste  
[PASS] Linux Ctrl+Shift+V paste  
[PASS] Windows toolbar Paste  
[PASS] Windows right-click paste  
[PASS] Windows Ctrl+Shift+V paste  
[PASS] macOS toolbar Paste  
[PASS] macOS right-click paste  
[PASS] macOS Command+V paste  
[PASS] UTF-8 paste  
[PASS] Large paste around 900 KB  
[PASS] Ctrl+C remote interrupt behavior

## Cross-platform status

- Linux: active first-test platform for new checkpoints
- Windows 10/11: standalone deployment previously validated; smoke required after SFTP browser changes
- macOS Intel: DMG tester flow previously validated; smoke required after SFTP browser changes

## Required regression guardrails

- Do not regress terminal transport/paste behavior
- Do not regress known-host multi-key portability
- Do not regress Windows-safe KEX compatibility
- Do not log passwords, private keys, clipboard contents, terminal contents, file contents, or full config JSON
- Keep `dd-ssh.json` human-readable
- Keep diagnostic logging OFF by default
