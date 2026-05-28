# Testcase — dev 0.1.8.1.1

Checkpoint: `dev 0.1.8.1.1` — Queue selected UI consolidation.

## Basic validation

- [ ] About dialog shows `dev 0.1.8.1.1`.
- [ ] Existing SSH terminal still opens.
- [ ] Existing single-file download still works.
- [ ] Existing single-file upload still works.
- [ ] Queue file upload/download still works.
- [ ] Retry selected still works.

## Folder upload

- [ ] Create a small local folder with at least one nested subfolder.
- [ ] Select the local folder.
- [ ] Click `Queue upload`.
- [ ] Confirm recursive queueing.
- [ ] Queue receives folder-create and file upload items.
- [ ] Start queue.
- [ ] Remote folder structure is created.
- [ ] Nested files are uploaded.

## Folder download

- [ ] Select a small remote folder.
- [ ] Click `Queue download`.
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


## Queue selected consolidation testcase

[ ] Local panel shows `Queue upload` only for queueing selected local items.  
[ ] Remote panel shows `Queue download` only for queueing selected remote items.  
[ ] Old separate folder queue buttons are not visible.  
[ ] Local file selection queues direct upload item(s).  
[ ] Local folder selection asks for recursive confirmation and expands into queue item(s).  
[ ] Mixed local file + folder selection works.  
[ ] Remote file selection queues direct download item(s).  
[ ] Remote folder selection asks for recursive confirmation and expands into queue item(s).  
[ ] Mixed remote file + folder selection works.  
[ ] Start queue, Retry selected, Overwrite all / Skip all, and exit safety still work.  
