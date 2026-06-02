# Testcase — dev 0.1.8.6.4

## Goal

Validate that queue prompts follow the actual queue order even when destructive delete items are present.

## Steps

1. Open DD-SSH and confirm About shows `dev 0.1.8.6.4`.
2. Open File Manager for a test session.
3. Ensure a test file exists both locally and remotely.
4. Queue the local file for upload to the same remote filename.
5. Queue the same local file for local delete.
6. Start queue.
7. Expected: remote overwrite confirmation appears first.
8. After upload completes, expected: local delete confirmation appears when the queue reaches the delete item.
9. Repeat the mirror case: queue download first, then remote delete.
10. Expected: local overwrite confirmation appears before remote delete confirmation.
11. Reject a delete confirmation. Expected: the delete item remains Pending and the queue stops before deleting.
12. Confirm upload/download/delete logging remains free of secrets and file contents when diagnostic logging is enabled.
