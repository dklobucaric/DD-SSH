# Testcase — dev 0.1.7.0

**Checkpoint:** dev 0.1.7.0 — Andromeda
**Type:** Terminal transport hardening.

## Purpose

Validate that terminal input/output remains functional after changing the terminal transport path to be byte-stream safer and partial-write aware.

## Test 1 — Basic terminal open

1. Start DD-SSH.
2. Open a known-good saved xterm.js session.
3. Confirm the terminal opens normally.
4. Run:

```bash
whoami
pwd
clear
stty size
```

Expected: output displays normally and Session Traffic updates.

## Test 2 — Fullscreen terminal apps

Run at least two of:

```bash
htop
top
nano /tmp/dd-ssh-0170-test.txt
vim /tmp/dd-ssh-0170-test.txt
```

Expected: keyboard input, redraw, Ctrl+C/exit, and reconnect/disconnect behavior remain OK.

## Test 3 — UTF-8 / special characters

Run:

```bash
printf 'UTF-8 test: čćžšđ ČĆŽŠĐ € 🚀\n'
```

Expected: UTF-8 text is not corrupted by terminal chunking.

## Test 4 — Output-heavy command

Run one of:

```bash
find /usr -maxdepth 3 -type f 2>/dev/null | head -1000
seq 1 5000
```

Expected: terminal remains responsive, output renders, RX traffic increases, and no crash occurs.

## Test 5 — Large paste / large input

Paste a larger multi-line text block, ideally 200–300 KB of code/text, into a safe shell context such as:

```bash
cat > /tmp/dd-ssh-large-paste.txt
# paste content
# finish with Ctrl+D from keyboard if needed
wc -c /tmp/dd-ssh-large-paste.txt
```

Expected: DD-SSH does not truncate the paste. TX traffic increases according to the sent byte count.

## Test 6 — Interrupt still works

Run:

```bash
sleep 30
```

Press keyboard Ctrl+C inside the terminal.

Expected: remote command is interrupted. The toolbar Copy button still copies selected text only.

## Test 7 — Diagnostic logging

Enable Settings → Enable diagnostic logging, then repeat a short connect/disconnect.

Expected: logs contain connection and traffic lifecycle lines, but no terminal input/output, password, private key content, or pasted text.

## Test 8 — Cross-platform smoke

Repeat Tests 1–4 on Linux, Windows standalone, and macOS DMG where practical.
