#pragma once

#include <QMainWindow>

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
    void addWelcomeTab();
    void showConnectDialog();

    QListWidget *m_sessionList = nullptr;
    QTabWidget *m_tabs = nullptr;
};
