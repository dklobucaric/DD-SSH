#include "MainWindow.h"
#include "ConnectDialog.h"
#include "ssh/SshSession.h"

#include <QAction>
#include <QApplication>
#include <QFontDatabase>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>
#include <QTextEdit>
#include <QToolBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("DD-SSH");

    setupMenus();
    setupToolbar();
    setupCentralLayout();

    statusBar()->showMessage("DD-SSH launch pad ready");

    resize(1100, 700);
}

void MainWindow::setupMenus()
{
    auto *fileMenu = menuBar()->addMenu("&File");

    auto *newSessionAction = fileMenu->addAction("New Session");
    connect(newSessionAction, &QAction::triggered, this, [this]() {
        showConnectDialog();
    });

    fileMenu->addSeparator();

    auto *exitAction = fileMenu->addAction("Exit");
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    auto *sessionMenu = menuBar()->addMenu("&Session");

    auto *connectAction = sessionMenu->addAction("Connect");
    connect(connectAction, &QAction::triggered, this, [this]() {
        showConnectDialog();
    });

    auto *toolsMenu = menuBar()->addMenu("&Tools");

    auto *multiExecAction = toolsMenu->addAction("Multi-Exec");
    connect(multiExecAction, &QAction::triggered, this, [this]() {
        statusBar()->showMessage("Multi-Exec placeholder clicked");
    });

    auto *helpMenu = menuBar()->addMenu("&Help");

    auto *aboutAction = helpMenu->addAction("About DD-SSH");
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(
            this,
            "About DD-SSH",
            "DD-SSH\n\n"
            "A clean cross-platform SSH client and session manager.\n\n"
            "Current phase: first SSH handshake skeleton.\n\n"
            "libssh version: " + SshSession::libsshVersion()
        );
    });
}

void MainWindow::setupToolbar()
{
    auto *toolbar = addToolBar("Main Toolbar");
    toolbar->setMovable(false);

    auto *newSessionAction = toolbar->addAction("New Session");
    connect(newSessionAction, &QAction::triggered, this, [this]() {
        showConnectDialog();
    });

    auto *connectAction = toolbar->addAction("Connect");
    connect(connectAction, &QAction::triggered, this, [this]() {
        showConnectDialog();
    });

    toolbar->addSeparator();

    auto *multiExecAction = toolbar->addAction("Multi-Exec");
    connect(multiExecAction, &QAction::triggered, this, [this]() {
        statusBar()->showMessage("Multi-Exec placeholder clicked");
    });

    toolbar->addSeparator();

    auto *settingsAction = toolbar->addAction("Settings");
    connect(settingsAction, &QAction::triggered, this, [this]() {
        statusBar()->showMessage("Settings placeholder clicked");
    });
}

void MainWindow::setupCentralLayout()
{
    auto *splitter = new QSplitter(Qt::Horizontal, this);

    m_sessionList = new QListWidget(splitter);
    m_sessionList->setMinimumWidth(240);
    m_sessionList->setMaximumWidth(360);

    m_sessionList->addItem("DD-Lab / Nextcloud Backend");
    m_sessionList->addItem("DD-Lab / Zabbix");
    m_sessionList->addItem("Lab / Test VM");
    m_sessionList->addItem("Local / Raspberry Pi");

    m_tabs = new QTabWidget(splitter);
    m_tabs->setTabsClosable(true);
    m_tabs->setMovable(true);

    connect(m_tabs, &QTabWidget::tabCloseRequested, this, [this](int index) {
        QWidget *widget = m_tabs->widget(index);
        m_tabs->removeTab(index);
        widget->deleteLater();

        if (m_tabs->count() == 0) {
            addWelcomeTab();
        }
    });

    connect(m_sessionList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        auto *terminalPlaceholder = new QTextEdit(this);
        terminalPlaceholder->setReadOnly(true);
        terminalPlaceholder->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        terminalPlaceholder->setPlainText(
            "DD-SSH terminal placeholder\n\n"
            "Selected session:\n" + item->text() + "\n\n"
            "Next milestone:\n"
            "- real session manager\n"
            "- libssh authentication\n"
            "- terminal frontend\n"
        );

        const int tabIndex = m_tabs->addTab(terminalPlaceholder, item->text());
        m_tabs->setCurrentIndex(tabIndex);

        statusBar()->showMessage("Opened placeholder tab for " + item->text());
    });

    splitter->addWidget(m_sessionList);
    splitter->addWidget(m_tabs);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    setCentralWidget(splitter);

    addWelcomeTab();
}

void MainWindow::addWelcomeTab()
{
    auto *welcome = new QTextEdit(this);
    welcome->setReadOnly(true);
    welcome->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    welcome->setPlainText(
        "DD-SSH\n\n"
        "UI layout skeleton is alive.\n\n"
        "Left side: session list placeholder\n"
        "Right side: terminal tabs placeholder\n\n"
        "Double-click a session on the left to open a placeholder tab.\n\n"
        "Current milestone:\n"
        "- first SSH handshake skeleton\n\n"
        "Next milestone:\n"
        "- host key fingerprint\n"
        "- password/private-key auth\n"
        "- real terminal channel\n"
    );

    m_tabs->addTab(welcome, "Welcome");
}

void MainWindow::showConnectDialog()
{
    ConnectDialog dialog(this);

    if (dialog.exec() != QDialog::Accepted) {
        statusBar()->showMessage("Connection dialog cancelled");
        return;
    }

    const QString tabTitle =
        dialog.username() + "@" + dialog.host() + ":" + QString::number(dialog.port());

    statusBar()->showMessage("Testing SSH handshake with " + tabTitle + "...");
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();

    const SshHandshakeResult handshake = SshSession::testHandshake(
        dialog.host(),
        dialog.port(),
        dialog.username()
    );

    QApplication::restoreOverrideCursor();

    const QString authLabel =
        dialog.authType() == ConnectDialog::AuthType::Password
            ? "Password"
            : "Private key";

    const QString secretInfo =
        dialog.authType() == ConnectDialog::AuthType::Password
            ? "Password: entered, hidden from display"
            : "Private key: " + dialog.keyPath();

    QString output;

    output += "DD-SSH manual connection test\n\n";
    output += "Host: " + dialog.host() + "\n";
    output += "Port: " + QString::number(dialog.port()) + "\n";
    output += "Username: " + dialog.username() + "\n";
    output += "Auth type: " + authLabel + "\n";
    output += secretInfo + "\n\n";

    output += "SSH handshake result:\n";

    if (handshake.success) {
        output += "Status: SUCCESS\n";
        output += "Message: " + handshake.message + "\n";
        output += "Server banner: " + handshake.serverBanner + "\n\n";
        output += "Authentication was NOT attempted yet.\n\n";
        output += "Next milestone:\n";
        output += "- host key fingerprint\n";
        output += "- known-host decision flow\n";
        output += "- password/private-key authentication\n";
    } else {
        output += "Status: FAILED\n";
        output += "Message: " + handshake.message + "\n";
        output += "libssh error code: " + QString::number(handshake.sshErrorCode) + "\n";
        output += "Error: " + handshake.error + "\n\n";
        output += "Check:\n";
        output += "- host/IP is correct\n";
        output += "- port is correct\n";
        output += "- SSH server is reachable\n";
        output += "- firewall allows connection\n";
    }

    auto *terminalPlaceholder = new QTextEdit(this);
    terminalPlaceholder->setReadOnly(true);
    terminalPlaceholder->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    terminalPlaceholder->setPlainText(output);

    const int tabIndex = m_tabs->addTab(terminalPlaceholder, tabTitle);
    m_tabs->setCurrentIndex(tabIndex);

    if (handshake.success) {
        statusBar()->showMessage("SSH handshake successful for " + tabTitle);
    } else {
        statusBar()->showMessage("SSH handshake failed for " + tabTitle);
    }
}
