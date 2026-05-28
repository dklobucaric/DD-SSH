# Testcase — dev 0.1.8.0.4

Checkpoint: `dev 0.1.8.0.4` — Transfer queue stabilization polish.

## Smoke checklist

- [ ] About dialog shows `dev 0.1.8.0.4`.
- [ ] File Manager opens from a saved session.
- [ ] Local and remote panels list files normally.
- [ ] Queue 2–3 downloads and start the queue.
- [ ] While the queue is running, queue control buttons are disabled.
- [ ] While the queue is running, local/remote navigation buttons and path fields are disabled.
- [ ] While the queue is running, local/remote file panels are disabled.
- [ ] Close DD-SSH while a queue is running; exit dialog lists SFTP transfer queue work.
- [ ] Choose Cancel in the exit dialog; DD-SSH remains open.
- [ ] Queue one or more files but do not start; close DD-SSH; exit dialog lists pending SFTP transfer queue work.
- [ ] Start queue with no Pending items; DD-SSH shows a clear no-pending-items message.
- [ ] `Retry selected` moves selected finished/skipped/cancelled items back to Pending.
- [ ] Existing immediate Download selected now still works.
- [ ] Existing immediate Upload selected now still works.
- [ ] Terminal tabs still connect.
- [ ] Paste and Ctrl+C still work.

## Regression guard

Do not accept this checkpoint if queue controls can mutate the queue while a queue run is active, or if closing DD-SSH with pending/running queue items does not warn the user.
