# Testcase — dev 0.1.8.6

Checkpoint: `dev 0.1.8.6` — File Manager delete UI polish.

## 1. UI smoke

1. Start DD-SSH.
2. Confirm About shows `dev 0.1.8.6`.
3. Right-click a saved session.
4. Confirm the menu item is `Open File Manager`.
5. Open the File Manager.
6. Confirm the local panel button is `Delete local`.
7. Confirm the remote panel button is `Delete remote`.

## 2. Local delete safety

1. Create a harmless local test file in the currently open local folder.
2. Select it in the local panel.
3. Click `Delete local`.
4. Confirm the enqueue warning.
5. Confirm the queue contains a `Delete local file` item.
6. Click `Start queue`.
7. First choose No/Cancel and confirm the item stays Pending.
8. Start again and confirm deletion.
9. Verify the local test file is removed and the local panel refreshes.

## 3. Remote delete regression

1. Upload or choose a harmless remote test file.
2. Select it in the remote panel.
3. Click `Delete remote`.
4. Confirm enqueue and start confirmations.
5. Verify the remote test file is removed and the remote panel refreshes.

## 4. Regression checks

- `Queue upload` still works.
- `Queue download` still works.
- Folder upload/download still works.
- Retry selected still works.
- Overwrite all / Skip all still works.
- Terminal open, paste, and Ctrl+C still work.
- Diagnostic logging does not include secrets, private keys, clipboard contents, terminal I/O, or file contents.
