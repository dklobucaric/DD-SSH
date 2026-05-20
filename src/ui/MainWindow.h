#pragma once

#include <QMainWindow>
#include <QPoint>
#include <QString>
#include <QStringList>

class QCloseEvent;
class QListWidget;
class QTabWidget;
class QToolBar;
struct ConfigInspection;

class MainWindow : public QMainWindow
{
public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void setupMenus();
    void setupToolbar();
    void setupCentralLayout();
    void loadSavedSessionsToSidebar();
    void addWelcomeTab();
    void applyAppTheme(const QString &themeName);
    void applyQuickToolbarVisibility(bool showQuickToolbar);
    void showConfigRecoveryWarningIfNeeded();
    bool showConfigRecoveryDialog(const ConfigInspection &inspection);
    void openConfigFolder();
    void exportConfig();
    void importConfig();
    void restoreLatestConfigBackup();
    QStringList activeSshTerminalNames() const;
    bool confirmExitWithActiveSshTerminals();
    void requestDisconnectForActiveSshTerminals();
    void showNewSessionDialog();
    void showManualConnectDialog();
    void showConnectDialog(bool newSavedSessionMode);
    void showSettingsDialog();
    void testSavedSession(const QString &sessionId);
    void openSavedSessionShell(const QString &sessionId);
    void openSavedSessionWebTerminal(const QString &sessionId);
    void openSavedSessionShellInternal(const QString &sessionId, bool useWebTerminal);
    void showSessionContextMenu(const QPoint &position);
    void editSelectedSession();
    void editSavedSession(const QString &sessionId);
    void deleteSavedSession(const QString &sessionId);

    QListWidget *m_sessionList = nullptr;
    QTabWidget *m_tabs = nullptr;
    QToolBar *m_mainToolBar = nullptr;
};
