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
        const QString aboutText =
            QStringLiteral("DD-SSH\n\n")
            + QStringLiteral("A clean cross-platform SSH client and session manager.\n\n")
            + QStringLiteral("Current phase: host key fingerprint display.\n\n")
            + QStringLiteral("libssh version: ")
            + SshSession::libsshVersion();

        QMessageBox::about(
            this,
            "About DD-SSH",
            aboutText
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

        const QString output =
            QStringLiteral("DD-SSH terminal placeholder\n\n")
            + QStringLiteral("Selected session:\n")
            + item->text()
            + QStringLiteral("\n\n")
            + QStringLiteral("Next milestone:\n")
            + QStringLiteral("- real session manager\n")
            + QStringLiteral("- libssh authentication\n")
            + QStringLiteral("- terminal frontend\n");

        terminalPlaceholder->setPlainText(output);

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
        "- host key fingerprint display\n\n"
        "Next milestone:\n"
        "- known-host decision flow\n"
        "- password/private-key authentication\n"
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
        dialog.username()
        + QStringLiteral("@")
        + dialog.host()
        + QStringLiteral(":")
        + QString::number(dialog.port());

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
            ? QStringLiteral("Password")
            : QStringLiteral("Private key");

    const QString secretInfo =
        dialog.authType() == ConnectDialog::AuthType::Password
            ? QStringLiteral("Password: entered, hidden from display")
            : QStringLiteral("Private key: ") + dialog.keyPath();

    QString output;

    output += QStringLiteral("DD-SSH manual connection test\n\n");
    output += QStringLiteral("Host: ") + dialog.host() + QStringLiteral("\n");
    output += QStringLiteral("Port: ") + QString::number(dialog.port()) + QStringLiteral("\n");
    output += QStringLiteral("Username: ") + dialog.username() + QStringLiteral("\n");
    output += QStringLiteral("Auth type: ") + authLabel + QStringLiteral("\n");
    output += secretInfo + QStringLiteral("\n\n");

    output += QStringLiteral("SSH handshake result:\n");

    if (handshake.success) {
        output += QStringLiteral("Status: SUCCESS\n");
        output += QStringLiteral("Message: ") + handshake.message + QStringLiteral("\n");
        output += QStringLiteral("Server banner: ") + handshake.serverBanner + QStringLiteral("\n");
        output += QStringLiteral("Host key type: ") + handshake.hostKeyType + QStringLiteral("\n");
        output += QStringLiteral("Host key fingerprint: ") + handshake.hostKeyFingerprint + QStringLiteral("\n\n");
        output += QStringLiteral("Authentication was NOT attempted yet.\n\n");
        output += QStringLiteral("Next milestone:\n");
        output += QStringLiteral("- known-host decision flow\n");
        output += QStringLiteral("- password/private-key authentication\n");
        output += QStringLiteral("- real terminal channel\n");
    } else {
        output += QStringLiteral("Status: FAILED\n");
        output += QStringLiteral("Message: ") + handshake.message + QStringLiteral("\n");
        output += QStringLiteral("libssh error code: ") + QString::number(handshake.sshErrorCode) + QStringLiteral("\n");
        output += QStringLiteral("Error: ") + handshake.error + QStringLiteral("\n\n");
        output += QStringLiteral("Check:\n");
        output += QStringLiteral("- host/IP is correct\n");
        output += QStringLiteral("- port is correct\n");
        output += QStringLiteral("- SSH server is reachable\n");
        output += QStringLiteral("- firewall allows connection\n");
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
