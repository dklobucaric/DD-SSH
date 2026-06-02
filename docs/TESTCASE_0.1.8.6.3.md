# Testcase — dev 0.1.8.6.3

## UI wording

[ ] Right-click a saved session.  
[ ] Menu contains `Open terminal`.  
[ ] Menu contains `Open file manager`.  
[ ] Menu contains `Run auth test`.  
[ ] Menu contains `Open fallback shell`.  
[ ] Menu contains `Edit session`.  
[ ] Menu contains `Delete session`.  

## About dialog

[ ] Help → About shows `dev 0.1.8.6.3`.  
[ ] `Current phase: Tester release polish for File Manager alpha.` appears below Codename and above Milestone.  
[ ] `Developer: DD-LAB vl. Dalibor Klobučarić` appears in the upper metadata area where the old Current phase line used to be.  

## File Manager toolbar

[ ] Local toolbar shows `Queue upload` before `Delete local`.  
[ ] Remote toolbar shows `Queue download` and `Delete remote`.  
[ ] Remote toolbar does not show `Download selected now`.  
[ ] Queue upload still queues selected local files/folders.  
[ ] Queue download still queues selected remote files/folders.  
[ ] Delete local queues supported local file/symlink/empty-folder delete items.  
[ ] Delete remote queues supported remote file/symlink/empty-folder delete items.  

## Regression

[ ] Terminal opens with `Open terminal`.  
[ ] File Manager opens with `Open file manager`.  
[ ] Fallback shell opens with `Open fallback shell`.  
[ ] Diagnostic logging still records transfer/delete events without secrets or file contents.  
[ ] Multiline paste into nano still preserves line breaks and indentation.  
