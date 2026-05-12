#pragma once

#include <QMainWindow>
#include <QPoint>
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
    void showSessionContextMenu(const QPoint &position);
    void editSavedSession(const QString &sessionId);
    void deleteSavedSession(const QString &sessionId);

    QListWidget *m_sessionList = nullptr;
    QTabWidget *m_tabs = nullptr;
};
