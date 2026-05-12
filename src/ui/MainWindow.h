#pragma once

#include <QMainWindow>
#include <QString>

class QListWidget;
class QTabWidget;

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
    void showConnectDialog();
    void testSavedSession(const QString &sessionId);

    QListWidget *m_sessionList = nullptr;
    QTabWidget *m_tabs = nullptr;
};
