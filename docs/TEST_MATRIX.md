## dev 0.1.8.9 — SFTP traffic monitor integration smoke test

[ ] About shows dev 0.1.8.9  
[ ] terminal traffic still updates for terminal tabs  
[ ] File Manager tab shows SFTP traffic context  
[ ] upload increases sent/up rate and total  
[ ] download increases received/down rate and total  
[ ] queued folder-expanded upload/download items update totals  
[ ] delete/mkdir/listing are not treated as bulk traffic  
[ ] diagnostic logging privacy rules still hold  

## dev 0.1.8.8.1 — Settings dialog General page polish smoke test

[ ] About shows dev 0.1.8.8.1  
[ ] Tools → Settings opens a readable dialog on Linux/Windows/macOS  
[ ] Category sidebar shows General, Diagnostics, Appearance, Terminal, Config safety, Security note  
[ ] Each category page uses left-aligned/readable controls without clipped labels  
[ ] Config path is visible in a read-only field  
[ ] Copy path copies the dd-ssh.json path to the clipboard  
[ ] Open folder opens the config folder  
[ ] Diagnostic logging checkbox still saves and toggles logging  
[ ] App theme setting still saves and applies after OK  
[ ] Terminal font family/size still save for newly opened terminals  
[ ] Config backup settings still save  
[ ] Cancel closes without saving changes  
[ ] Existing terminal, paste, SFTP File Manager, queue/delete, and packaging behavior still work  

# DD-SSH Test Matrix

## dev 0.1.8.7 — Release/tester packaging polish smoke test

[ ] About shows dev 0.1.8.7  
[ ] About shows milestone `Release/tester packaging polish`  
[ ] Welcome screen lists current SSH terminal and SFTP File Manager alpha features  
[ ] README current status shows dev 0.1.8.7  
[ ] `docs/FEATURE_AUDIT_0.1.8.7.md` exists and lists current features/limitations  
[ ] `docs/RELEASE_TESTER_PACKAGING_0.1.8.7.md` exists and lists Linux/Windows/macOS packaging commands  
[ ] `docs/TESTER_CHECKLIST_0.1.8.7.md` exists for third-party testers  
[ ] Linux `.deb` helper produces a `0.1.8.7` artifact name  
[ ] macOS `.dmg` helper produces a `0.1.8.7` artifact name and otool report  
[ ] Windows portable ZIP helper produces `DD-SSH-0.1.8.7-Windows-x86_64-portable.zip`  
[ ] Checksum helpers produce `dist/SHA256SUMS`  
[ ] Terminal opens and multiline paste still works  
[ ] File Manager queue upload/download/delete still works  
[ ] Diagnostic logging remains OFF by default and logs no secrets/file contents when enabled  

**Checkpoint:** dev 0.1.8.7 — Andromeda  
**Milestone:** Release/tester packaging polish  
**Phase:** Release/tester packaging polish for File Manager alpha

## dev 0.1.8.6.4 — Queue delete confirmation ordering polish smoke test

[ ] About shows dev 0.1.8.6.4  
[ ] Saved-session context menu shows `Open terminal`  
[ ] Saved-session context menu shows `Open file manager`  
[ ] Saved-session context menu shows `Run auth test`  
[ ] Saved-session context menu shows `Open fallback shell`  
[ ] About dialog shows Developer in the upper metadata area, then Version, Codename, Current phase, and Milestone  
[ ] About dialog shows `Developer: DD-LAB vl. Dalibor Klobučarić`  
[ ] Local File Manager toolbar shows `Queue upload` before `Delete local`  
[ ] Remote File Manager toolbar shows `Queue download` and `Delete remote`; no visible `Download selected now` button  
[ ] Queue upload/download/delete, folder queue, Retry selected, Overwrite all / Skip all, diagnostic logging, and terminal paste still work  

**Checkpoint:** dev 0.1.8.6.4 — Andromeda  
**Milestone:** Queue delete confirmation ordering polish  
**Phase:** Tester release polish for File Manager alpha

## dev 0.1.8.6.2 — terminal paste newline safety smoke test

[ ] About shows dev 0.1.8.6.2  
[ ] Open an SSH terminal and run `nano /tmp/dd-ssh-paste-test.yml`  
[ ] Paste a multiline YAML/config block using toolbar Paste  
[ ] Pasted text keeps one line per source line; nano must not show `Justified paragraph`  
[ ] Indentation is preserved for nested YAML sections  
[ ] Save and inspect with `cat -n /tmp/dd-ssh-paste-test.yml`  
[ ] Repeat with right-click paste / Ctrl+Shift+V / Command+V where applicable  
[ ] Single-line paste still works  
[ ] Ctrl+C remote interrupt still works  
[ ] File Manager queue/delete smoke still works  

## dev 0.1.8.6.1 — macOS/Linux artifact version polish smoke test

[ ] About shows dev 0.1.8.6.1  
[ ] macOS build still succeeds with `./scripts/macos-build-release.sh`  
[ ] macOS deploy prints `Package version: 0.1.8.6.1`  
[ ] macOS deploy creates `dist/macos/DD-SSH-0.1.8.6.1-macOS-x86_64.dmg` by default  
[ ] macOS dependency audit report is named `DD-SSH-0.1.8.6.1-macOS-x86_64-otool-report.txt` by default  
[ ] `DD_SSH_MACOS_VERSION=custom ./scripts/macos-deploy-release.sh` still overrides the artifact version if needed  
[ ] Linux Debian package script prints the detected package version and no longer defaults to stale `0.1.7.1`  
[ ] File Manager, terminal, queue upload/download, local/remote delete, and diagnostic logging still work  

## dev 0.1.8.6 — local/remote delete UI polish smoke test

[ ] About shows dev 0.1.8.6  
[ ] Saved-session context menu shows `Open File Manager`  
[ ] Local panel shows `Delete local` instead of `Upload selected now`  
[ ] Remote panel shows `Delete remote` instead of `Queue delete`  
[ ] Local regular test file can be queued for delete after confirmation  
[ ] Queue start asks for a second local destructive confirmation  
[ ] Cancelling local delete start leaves item Pending  
[ ] Confirming local delete removes the local test file and refreshes the local panel  
[ ] Local empty folder delete works or fails safely if not empty/permission denied  
[ ] Remote delete behavior from dev 0.1.8.5 still works  
[ ] Diagnostic logging records local/remote delete events without secrets or file contents  
[ ] Queue upload/download, folder transfer, Retry selected, Overwrite all / Skip all, and terminal paste/Ctrl+C still work  

## dev 0.1.8.5 — remote queue delete experiment smoke test

[ ] About shows dev 0.1.8.5  
[ ] Remote panel shows `Queue delete`  
[ ] Select a harmless remote test file and queue it for delete  
[ ] Enqueue confirmation warns that delete is destructive  
[ ] Queue table shows a `Delete remote file` item  
[ ] Start queue asks for a second destructive confirmation before delete runs  
[ ] Cancelling the start confirmation leaves delete item(s) Pending  
[ ] Confirming delete removes the remote test file and refreshes the remote panel  
[ ] Empty remote folder delete may succeed; non-empty folder delete fails safely  
[ ] Diagnostic logging records queued/started/completed/failed delete events without secrets or file contents  
[ ] Upload/download/overwrite metadata dialogs still work  
[ ] Terminal and paste/Ctrl+C still work  

## dev 0.1.8.3.1 — SFTP logging polish smoke test

[ ] About shows dev 0.1.8.3.1  
[ ] Diagnostic logging is OFF by default  
[ ] Enable diagnostic logging in Settings  
[ ] Upload a file that does not already exist remotely  
[ ] Log contains `SFTP upload preflight started` before connection/auth metadata  
[ ] Log contains `SFTP file upload started` only when the transfer phase begins  
[ ] Queue/upload a file that already exists remotely and choose Overwrite / Overwrite all  
[ ] Log contains `SFTP upload target exists before transfer` before the overwrite decision  
[ ] Log no longer suggests a duplicate data-transfer start before overwrite approval  
[ ] Logs still do not contain passwords, private keys, plaintext secret values, terminal input/output, clipboard contents, or file contents  
[ ] Queue, upload/download, folder transfer, Retry selected, Overwrite all / Skip all, terminal, and paste/Ctrl+C still work  

## dev 0.1.8.3 — file transfer logging and diagnostics smoke test

[ ] About shows dev 0.1.8.3  
[ ] Diagnostic logging is OFF by default  
[ ] Enable diagnostic logging in Settings  
[ ] Help → Open Log Folder opens the expected per-user log directory  
[ ] Single-file download writes useful start/success metadata to the log  
[ ] Single-file upload writes useful start/success metadata to the log  
[ ] Cancelled download/upload writes WARN metadata, not file contents  
[ ] Queue start and queue finish are logged with counts  
[ ] Queue item Done/Failed/Cancelled/Skipped states are logged  
[ ] Queue Overwrite all / Skip all decisions are logged  
[ ] Retry selected is logged with requeued counts  
[ ] Folder queue confirmation and folder scan summary events are logged  
[ ] Logs do not contain passwords, private keys, plaintext secret values, terminal input/output, clipboard contents, or file contents  
[ ] Terminal, paste/Ctrl+C, File Manager, queue, folder upload/download, and safety behavior still work  
[ ] Disable diagnostic logging and confirm no new transfer events are appended after disabling  

## dev 0.1.8.2 — file manager safety polish smoke test

[ ] About shows dev 0.1.8.2  
[ ] Welcome tab mentions the tester-ready file transfer baseline  
[ ] Existing SSH terminal still opens  
[ ] Existing paste/Ctrl+C behavior still works  
[ ] File Manager opens with local and remote panels  
[ ] Local panel still shows `Queue upload`  
[ ] Remote panel still shows `Queue download`  
[ ] Single-file upload/download still works  
[ ] Folder upload/download still works with a small test folder  
[ ] Folder queue confirmation includes safety rules and the item limit  
[ ] Folder queue scan summary appears after selecting a folder  
[ ] Queue completion refreshes local and remote panels  
[ ] Queue remains sequential, one item at a time  
[ ] Retry selected still works  
[ ] Overwrite all / Skip all still works  
[ ] Exit safety still reports open terminal/file manager/queue work  
[ ] No SFTP Session Traffic integration is expected yet  
[ ] Tester checklist exists: `docs/TESTER_CHECKLIST_0.1.8.1.2.md`  
[ ] Known limitations are clear about plaintext secrets and experimental folder transfer  

## dev 0.1.8.1.1 — queue selected UI consolidation smoke test

[ ] About shows dev 0.1.8.1.1  
[ ] File Manager opens with local and remote panels  
[ ] Local panel shows a single `Queue upload` button for queueing selected local files/folders  
[ ] Remote panel shows a single `Queue download` button for queueing selected remote files/folders  
[ ] Separate `Queue folder upload` / `Queue folder download` buttons are not visible  
[ ] Mixed local file + folder selection queues file(s) directly and asks for recursive folder confirmation  
[ ] Mixed remote file + folder selection queues file(s) directly and asks for recursive folder confirmation  
[ ] Local folder upload through `Queue upload` creates the remote folder and uploads nested files  
[ ] Remote folder download through `Queue download` creates the local folder and downloads nested files  
[ ] Empty destination folders are created as queue directory items  
[ ] Symlinks/special entries are skipped, not followed  
[ ] Queue still runs sequentially, one item at a time  
[ ] Retry selected still works on Done/Failed/Cancelled/Skipped items  
[ ] Overwrite all / Skip all still works when folder transfer hits existing files  
[ ] Cancel behavior remains clear for upload/download items  
[ ] Terminal, paste/Ctrl+C, and existing SFTP single-file transfer behavior still work  

## dev 0.1.8.0.4.1 — transfer queue stabilization polish smoke test

[ ] About shows dev 0.1.8.0.4.1  
[ ] File Manager opens from a saved session  
[ ] Local panel lists local files  
[ ] Remote panel lists remote SFTP files  
[ ] Queue table shows Status / Direction / Name / Size / Source / Target  
[ ] Queue one or more files and start the queue  
[ ] While the queue is running, Start queue / Retry selected / Remove selected / Clear finished are disabled  
[ ] While the queue is running, local/remote path fields and navigation controls are disabled  
[ ] While the queue is running, local and remote file panels are disabled  
[ ] Closing DD-SSH with a running queue shows exit confirmation including SFTP transfer queue work  
[ ] Closing DD-SSH with pending queue items shows exit confirmation including SFTP transfer queue work  
[ ] Cancel in the exit dialog keeps DD-SSH open  
[ ] Start queue with no Pending items shows a clear no-pending-items message  
[ ] `Retry selected` still moves Done/Failed/Cancelled/Skipped items back to Pending  
[ ] Existing queue Overwrite / Skip / Overwrite all / Skip all decisions still work  
[ ] Existing `Download selected now` still downloads one selected remote file  
[ ] Existing `Upload selected now` still uploads one selected local file  
[ ] Terminal xterm.js still connects  
[ ] Paste / Ctrl+C still work  
[ ] Terminal Session Traffic still works for terminal tabs only; SFTP queue traffic is not expected yet  

## dev 0.1.8.0.3 — transfer queue retry-selected polish smoke test

[ ] About shows dev 0.1.8.0.3  
[ ] File Manager opens from a saved session  
[ ] Local panel lists local files  
[ ] Remote panel lists remote SFTP files  
[ ] Queue table shows Status / Direction / Name / Size / Source / Target  
[ ] `Retry selected` button is visible between `Start queue` and `Remove selected`  
[ ] Queue multiple remote downloads and run them to Done  
[ ] Select one or more Done queue items and click `Retry selected`  
[ ] Selected Done items return to Pending  
[ ] `Start queue` processes the requeued download items again  
[ ] Overwrite / Skip / Overwrite all / Skip all decisions still work for retried download items  
[ ] Queue multiple local uploads and run them to Done  
[ ] Select one or more Done upload items and click `Retry selected`  
[ ] Selected Done upload items return to Pending  
[ ] `Start queue` processes the requeued upload items again  
[ ] Cancelled, Failed, and Skipped items can be moved back to Pending with `Retry selected`  
[ ] Pending items remain Pending if selected for retry  
[ ] Running items cannot be retried while the queue is active  
[ ] `Remove selected` still removes selected non-running queue items  
[ ] `Clear finished` still removes Done/Failed/Cancelled/Skipped queue items  
[ ] Queue remains sequential; parallel transfer is not expected  
[ ] Existing `Download selected now` still downloads one selected remote file  
[ ] Existing `Upload selected now` still uploads one selected local file  
[ ] Terminal xterm.js still connects  
[ ] Paste / Ctrl+C still work  
[ ] Terminal Session Traffic still works for terminal tabs only; SFTP queue traffic is not expected yet  

## dev 0.1.8.0.2 — transfer queue overwrite-all polish smoke test

[ ] About shows dev 0.1.8.0.2  

[ ] Queue overwrite dialog offers Overwrite, Skip, Overwrite all, Skip all, and Cancel queue  
[ ] Overwrite all applies to remaining existing local download targets in the current queue run  
[ ] Overwrite all applies to remaining existing remote upload targets in the current queue run  
[ ] Skip all skips remaining matching overwrite conflicts in the current queue run  
[ ] Queue upload overwrite prompt is visible on top, not hidden behind progress  
[ ] Queue download overwrite prompt is visible before progress starts  
[ ] No/Skip marks item Skipped and queue continues  
[ ] Cancel marks item Cancelled and stops queue cleanly  
[ ] File Manager opens from a saved session  
[ ] Local panel lists local files  
[ ] Remote panel lists remote SFTP files  
[ ] Existing `Download selected now` still downloads one selected remote file  
[ ] Existing `Upload selected now` still uploads one selected local file  
[ ] Selecting multiple remote files and clicking `Queue download(s)` adds multiple queue rows  
[ ] Selecting multiple local files and clicking `Queue upload(s)` adds multiple queue rows  
[ ] Queue table shows Status / Direction / Name / Size / Source / Target  
[ ] `Start queue` processes items one at a time  
[ ] Download queue items end as Done and appear in the local folder  
[ ] Upload queue items end as Done and appear in the remote folder  
[ ] Existing local overwrite prompt appears for queued downloads when needed  
[ ] Existing remote overwrite prompt/skip behavior appears for queued uploads when needed  
[ ] Cancelling a queued download reports that the local target was not replaced  
[ ] Cancelling a queued upload reports that a partial remote file may remain  
[ ] `Remove selected` removes selected non-running queue items  
[ ] `Clear finished` removes Done/Failed/Cancelled/Skipped queue items  
[ ] Folders are skipped/refused; folder transfer is not implemented yet  
[ ] Queue is sequential; parallel transfer is not expected  
[ ] Terminal xterm.js still connects  
[ ] Paste / Ctrl+C still work  
[ ] Terminal Session Traffic still works for terminal tabs only; SFTP queue traffic is not expected yet  

## dev 0.1.7.8 — transfer progress/cancel polish smoke test

[ ] About shows dev 0.1.7.8  
[ ] File Manager opens with local and remote panels  
[ ] Download selected still works for a small file  
[ ] Download progress dialog shows transferred size, total size, percent, speed, and elapsed time  
[ ] Download completion dialog shows downloaded formatted size, raw byte count, elapsed time, and average speed  
[ ] Cancelling a download shows a clear cancel message and does not replace the local target file  
[ ] Upload selected still works for a small file  
[ ] Upload progress dialog shows transferred size, total size, percent, speed, and elapsed time  
[ ] Upload completion dialog shows uploaded formatted size, raw byte count, elapsed time, and average speed  
[ ] Cancelling an upload shows a clear cancel message and warns that a partial remote file may remain  
[ ] Existing overwrite prompts still appear  
[ ] Folder upload/download are still rejected clearly  
[ ] Terminal xterm.js session still opens and paste/Ctrl+C still work  
[ ] Session Traffic for terminal tabs still works  
[ ] SFTP upload/download traffic is expected in the live traffic widget for active File Manager tabs  

## dev 0.1.7.7 — single-file SFTP upload smoke test

[ ] About shows dev 0.1.7.7  
[ ] File Manager opens with Local files and Remote files panels  
[ ] Remote listing still works  
[ ] Download selected still works for a small file  
[ ] Select one local file and click Upload selected  
[ ] Upload progress dialog appears  
[ ] Uploaded file appears in the current remote folder after refresh  
[ ] Remote overwrite prompt appears when uploading the same filename again  
[ ] Cancel on overwrite does not overwrite the remote file  
[ ] Folder upload is rejected clearly  
[ ] Terminal xterm.js session still opens and paste/Ctrl+C still work  
[ ] Session Traffic for terminal tabs still works  
[ ] SFTP upload/download traffic is expected in the live traffic widget for active File Manager tabs  

## dev 0.1.7.6.1 — single-file SFTP download polish smoke test

[ ] About shows dev 0.1.7.6.1  
[ ] File Manager opens with local and remote panels  
[ ] Remote SFTP listing still works  
[ ] Remote `Name` sorting still works  
[ ] Remote `Type` sorting still works  
[ ] Remote `Size` sorting orders by raw byte count across B / KB / MB / GB values  
[ ] Select one remote file and click `Download selected`  
[ ] Download still writes the file into the current local folder  
[ ] Existing local target still shows overwrite warning  
[ ] Progress dialog still appears during download  
[ ] Completion message shows `Downloaded: <formatted size> (<raw bytes> bytes)`  
[ ] Local panel refreshes after successful download  
[ ] Attempting to download a remote folder is still refused clearly  
[ ] Existing xterm.js terminal, paste paths, Ctrl+C interrupt, known-host checks, and logging still work  

## dev 0.1.7.6 — single-file SFTP download smoke test

[ ] About shows dev 0.1.7.6  
[ ] Saved-session context menu shows `Open File Manager (download enabled)`  
[ ] File Manager opens with a local panel on the left and remote SFTP panel on the right  
[ ] Local panel starts at the user's home directory or a valid local path  
[ ] Remote panel lists a known SFTP-enabled server  
[ ] Select one remote file and click `Download selected`  
[ ] Download writes the file into the currently open local folder  
[ ] Existing local target shows an overwrite warning  
[ ] Cancel on overwrite leaves the local file unchanged  
[ ] Progress dialog appears during download  
[ ] Local panel refreshes after successful download  
[ ] Downloaded file size matches the remote file size  
[ ] Attempting to download a remote folder is refused clearly  
[ ] Upload/delete/rename/folder transfer/queue actions are not present yet  
[ ] Exit safety still lists open File Manager/SFTP tabs  
[ ] Existing xterm.js terminal, paste paths, Ctrl+C interrupt, known-host checks, and logging still work  
[ ] Session Traffic still reports terminal traffic only; SFTP transfer counters are not expected yet  

## dev 0.1.7.5 — local + remote read-only file manager smoke test

[PASS] Linux smoke test passed  
[PASS] macOS smoke test passed  
[PASS] Windows smoke test passed  

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
[ ] SFTP upload/download traffic is expected in the live Session Traffic monitor  
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
