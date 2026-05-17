#pragma once

#include <QMainWindow>
#include <QPoint>
#include <QString>

class QListWidget;
class QTabWidget;
struct ConfigInspection;

class MainWindow : public QMainWindow
{
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void setupMenus();
    void setupToolbar();
    void setupCentralLayout();
    void loadSavedSessionsToSidebar();
    void addWelcomeTab();
    void applyAppTheme(const QString &themeName);
    void showConfigRecoveryWarningIfNeeded();
    bool showConfigRecoveryDialog(const ConfigInspection &inspection);
    void openConfigFolder();
    void showConnectDialog();
    void showSettingsDialog();
    void testSavedSession(const QString &sessionId);
    void openSavedSessionShell(const QString &sessionId);
    void openSavedSessionWebTerminal(const QString &sessionId);
    void openSavedSessionShellInternal(const QString &sessionId, bool useWebTerminal);
    void showSessionContextMenu(const QPoint &position);
    void editSavedSession(const QString &sessionId);
    void deleteSavedSession(const QString &sessionId);

    QListWidget *m_sessionList = nullptr;
    QTabWidget *m_tabs = nullptr;
};
