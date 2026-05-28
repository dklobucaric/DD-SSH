# Testcase — dev 0.1.8.1

Checkpoint: `dev 0.1.8.1` — Folder transfer experiment.

## Basic validation

- [ ] About dialog shows `dev 0.1.8.1`.
- [ ] Existing SSH terminal still opens.
- [ ] Existing single-file download still works.
- [ ] Existing single-file upload still works.
- [ ] Queue file upload/download still works.
- [ ] Retry selected still works.

## Folder upload

- [ ] Create a small local folder with at least one nested subfolder.
- [ ] Select the local folder.
- [ ] Click `Queue folder upload`.
- [ ] Confirm recursive queueing.
- [ ] Queue receives folder-create and file upload items.
- [ ] Start queue.
- [ ] Remote folder structure is created.
- [ ] Nested files are uploaded.

## Folder download

- [ ] Select a small remote folder.
- [ ] Click `Queue folder download`.
- [ ] Confirm recursive queueing.
- [ ] Queue receives local-folder-create and file download items.
- [ ] Start queue.
- [ ] Local folder structure is created.
- [ ] Nested files are downloaded.

## Safety

- [ ] Symlinks/special files are skipped, not followed.
- [ ] Existing-file conflicts still show overwrite prompts.
- [ ] Overwrite all and Skip all still behave correctly.
- [ ] Cancel during file transfer still gives clear feedback.
- [ ] Exit safety still warns about running/pending queue work.
