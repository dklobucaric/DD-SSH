# DD-SSH Architecture

DD-SSH uses a layered architecture.

```text
UI Layer
- MainWindow
- Session sidebar
- Terminal tabs
- Settings dialog
- Multi-exec panel

Core Layer
- SessionManager
- ConfigManager
- KnownHostsManager
- MultiExecManager
- SyncManager

SSH Layer
- SshSession
- SshWorker
- AuthManager
- KeepAliveManager

Terminal Layer
- TerminalFrontend interface
- XtermJsTerminalFrontend
- Future native terminal frontend
```

## Threading

The GUI must stay responsive.

```text
GUI thread:
- Qt window
- tabs
- session list
- terminal rendering

Worker thread:
- SSH connect
- SSH authentication
- SSH read loop
- keep-alive
```

SSH read loops must never block the GUI thread.
