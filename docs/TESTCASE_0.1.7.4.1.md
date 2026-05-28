# Testcase — dev 0.1.7.4.1

Checkpoint: `dev 0.1.7.4.1 — Read-only SFTP browser bugfix polish`

## Version

- [ ] Help → About DD-SSH shows `dev 0.1.7.4.1`

## SFTP browser readability

- [ ] Right-click a saved session → `Open File Manager (read-only)`
- [ ] Browser opens and lists a known SFTP-enabled server
- [ ] Table rows are readable in dark theme
- [ ] There are no white alternating rows with unreadable white text
- [ ] Header and selection remain usable
- [ ] `↑ Up` button is visible and usable

## Exit safety

- [ ] Open one active SSH terminal tab
- [ ] Open one SFTP browser tab
- [ ] Press the application window close button
- [ ] Confirmation dialog lists SSH terminal session(s)
- [ ] Confirmation dialog lists SFTP browser tab(s)
- [ ] Cancel keeps DD-SSH open
- [ ] Close and Exit disconnects active SSH terminals and exits

## Crowded tab bar

- [ ] Open enough terminal/SFTP tabs to overflow the tab bar
- [ ] Linux/Windows show tab navigation/scroll controls as before
- [ ] macOS is checked for improved scroll-button/overflow behavior
- [ ] If macOS still does not show native arrows, record it as platform UI behavior for later custom overflow/dropdown polish

## Regression

- [ ] Existing xterm.js terminal opens normally
- [ ] Paste paths still work
- [ ] Ctrl+C remote interrupt still works
- [ ] Session Traffic still works for terminal tabs
- [ ] SFTP traffic is not expected in Session Traffic yet
- [ ] Known-host prompts still behave correctly
- [ ] Diagnostic logging remains OFF by default

## Pass condition

This checkpoint passes when the read-only browser is usable/readable, exit safety includes both SSH and SFTP tabs, and terminal behavior from the 0.1.7.1 baseline remains stable on Linux, with Windows/macOS smoke tests completed.
