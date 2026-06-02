# DD-SSH dev 0.1.8.6.2 — Build and test

Goal: verify terminal paste newline safety for multiline YAML/config text, especially inside full-screen terminal applications such as nano.

## Linux build smoke

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

Expected: About shows `dev 0.1.8.6.2`.

## Paste smoke test

Open any saved SSH terminal session and run:

```bash
nano /tmp/dd-ssh-paste-test.yml
```

Paste a multiline YAML/config block using DD-SSH toolbar Paste, right-click paste, or the platform paste shortcut.

Expected:

- The pasted text keeps the same line structure as the clipboard.
- YAML indentation is preserved.
- nano must not show `Justified paragraph` after paste.
- Saving and inspecting with `cat -n /tmp/dd-ssh-paste-test.yml` shows one source line per pasted line.

## Regression smoke

- Single-line paste still works.
- Ctrl+C still interrupts remote commands.
- Terminal output rendering still works.
- File Manager queue upload/download/delete smoke still works.
