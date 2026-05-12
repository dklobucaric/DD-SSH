#include "MainWindow.h"
#include "ConnectDialog.h"
#include "core/KnownHostsManager.h"
#include "ssh/SshSession.h"

#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QFontDatabase>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
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
        KnownHostsManager knownHosts;

        const QString aboutText =
            QStringLiteral("DD-SSH\n\n")
            + QStringLiteral("A clean cross-platform SSH client and session manager.\n\n")
            + QStringLiteral("Current phase: SSH authentication test.\n\n")
            + QStringLiteral("libssh version: ")
            + SshSession::libsshVersion()
            + QStringLiteral("\n\nConfig file:\n")
            + knownHosts.configFilePath();

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
            + QStringLiteral("- terminal frontend\n")
            + QStringLiteral("- persistent SSH session object\n");

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
        "- SSH authentication test\n\n"
        "Next milestone:\n"
        "- save successful connection to JSON\n"
        "- persistent SSH session object\n"
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

    KnownHostsManager knownHosts;

    bool knownHostAllowed = false;
    QString knownHostDecision = QStringLiteral("Not checked");
    QString knownHostExtra;

    if (handshake.success) {
        const KnownHostsManager::CheckResult check = knownHosts.checkHost(
            dialog.host(),
            dialog.port(),
            handshake.hostKeyType,
            handshake.hostKeyFingerprint
        );

        if (check.status == KnownHostsManager::HostStatus::Trusted) {
            knownHostAllowed = true;
            knownHostDecision = QStringLiteral("TRUSTED - stored fingerprint matches current host key");

            QString saveError;
            knownHosts.trustHost(
                dialog.host(),
                dialog.port(),
                handshake.hostKeyType,
                handshake.hostKeyFingerprint,
                &saveError
            );

            if (!saveError.isEmpty()) {
                knownHostExtra = QStringLiteral("Warning while updating last_seen: ") + saveError;
            }
        } else if (check.status == KnownHostsManager::HostStatus::Unknown) {
            QMessageBox messageBox(this);
            messageBox.setIcon(QMessageBox::Warning);
            messageBox.setWindowTitle("Unknown SSH host");
            messageBox.setText("This SSH host is not trusted yet.");
            messageBox.setInformativeText(
                "Host: " + dialog.host() + ":" + QString::number(dialog.port()) + "\n"
                "Key type: " + handshake.hostKeyType + "\n"
                "Fingerprint: " + handshake.hostKeyFingerprint + "\n\n"
                "Do you want to trust this host?"
            );

            QAbstractButton *trustOnceButton = messageBox.addButton("Trust once", QMessageBox::AcceptRole);
            QAbstractButton *trustPermanentlyButton = messageBox.addButton("Trust permanently", QMessageBox::AcceptRole);
            QAbstractButton *cancelButton = messageBox.addButton("Cancel", QMessageBox::RejectRole);

            messageBox.setDefaultButton(qobject_cast<QPushButton *>(cancelButton));
            messageBox.exec();

            if (messageBox.clickedButton() == trustOnceButton) {
                knownHostAllowed = true;
                knownHostDecision = QStringLiteral("UNKNOWN - trusted once only");
            } else if (messageBox.clickedButton() == trustPermanentlyButton) {
                QString saveError;

                if (knownHosts.trustHost(
                        dialog.host(),
                        dialog.port(),
                        handshake.hostKeyType,
                        handshake.hostKeyFingerprint,
                        &saveError
                    )) {
                    knownHostAllowed = true;
                    knownHostDecision = QStringLiteral("UNKNOWN - trusted permanently and saved to config");
                } else {
                    knownHostAllowed = false;
                    knownHostDecision = QStringLiteral("UNKNOWN - could not save trusted host");
                    knownHostExtra = saveError;

                    QMessageBox::warning(
                        this,
                        "Could not save known host",
                        saveError
                    );
                }
            } else {
                knownHostAllowed = false;
                knownHostDecision = QStringLiteral("UNKNOWN - cancelled by user");
            }
        } else {
            QMessageBox messageBox(this);
            messageBox.setIcon(QMessageBox::Critical);
            messageBox.setWindowTitle("SSH host key changed");
            messageBox.setText("WARNING: The stored SSH host key does not match the current server key.");
            messageBox.setInformativeText(
                "Host: " + dialog.host() + ":" + QString::number(dialog.port()) + "\n\n"
                "Stored key type: " + check.storedKeyType + "\n"
                "Stored fingerprint: " + check.storedFingerprint + "\n\n"
                "Current key type: " + handshake.hostKeyType + "\n"
                "Current fingerprint: " + handshake.hostKeyFingerprint + "\n\n"
                "This may mean the server was reinstalled, DNS/IP changed, or a man-in-the-middle attack is possible."
            );

            QAbstractButton *replaceButton = messageBox.addButton("Replace stored key", QMessageBox::DestructiveRole);
            QAbstractButton *trustOnceButton = messageBox.addButton("Trust once", QMessageBox::AcceptRole);
            QAbstractButton *cancelButton = messageBox.addButton("Cancel", QMessageBox::RejectRole);

            messageBox.setDefaultButton(qobject_cast<QPushButton *>(cancelButton));
            messageBox.exec();

            if (messageBox.clickedButton() == replaceButton) {
                QString saveError;

                if (knownHosts.trustHost(
                        dialog.host(),
                        dialog.port(),
                        handshake.hostKeyType,
                        handshake.hostKeyFingerprint,
                        &saveError
                    )) {
                    knownHostAllowed = true;
                    knownHostDecision = QStringLiteral("CHANGED - stored key replaced by user");
                } else {
                    knownHostAllowed = false;
                    knownHostDecision = QStringLiteral("CHANGED - could not replace stored key");
                    knownHostExtra = saveError;

                    QMessageBox::warning(
                        this,
                        "Could not replace known host",
                        saveError
                    );
                }
            } else if (messageBox.clickedButton() == trustOnceButton) {
                knownHostAllowed = true;
                knownHostDecision = QStringLiteral("CHANGED - trusted once only");
            } else {
                knownHostAllowed = false;
                knownHostDecision = QStringLiteral("CHANGED - cancelled by user");
            }
        }
    }

    QString output;

    output += QStringLiteral("DD-SSH manual connection test\n\n");
    output += QStringLiteral("Host: ") + dialog.host() + QStringLiteral("\n");
    output += QStringLiteral("Port: ") + QString::number(dialog.port()) + QStringLiteral("\n");
    output += QStringLiteral("Username: ") + dialog.username() + QStringLiteral("\n");
    output += QStringLiteral("Auth type: ") + authLabel + QStringLiteral("\n");
    output += secretInfo + QStringLiteral("\n\n");

    output += QStringLiteral("SSH handshake result:\n");

    bool authAttempted = false;
    bool authSuccessful = false;
    SshAuthResult authResult;

    if (handshake.success) {
        output += QStringLiteral("Status: SUCCESS\n");
        output += QStringLiteral("Message: ") + handshake.message + QStringLiteral("\n");
        output += QStringLiteral("Server banner: ") + handshake.serverBanner + QStringLiteral("\n");
        output += QStringLiteral("Host key type: ") + handshake.hostKeyType + QStringLiteral("\n");
        output += QStringLiteral("Host key fingerprint: ") + handshake.hostKeyFingerprint + QStringLiteral("\n\n");

        output += QStringLiteral("Known-host result:\n");
        output += QStringLiteral("Decision: ") + knownHostDecision + QStringLiteral("\n");
        output += QStringLiteral("Config file: ") + knownHosts.configFilePath() + QStringLiteral("\n");

        if (!knownHostExtra.isEmpty()) {
            output += QStringLiteral("Extra: ") + knownHostExtra + QStringLiteral("\n");
        }

        output += QStringLiteral("\n");

        if (knownHostAllowed) {
            output += QStringLiteral("Known-host decision allows continuing.\n\n");

            const SshAuthMethod authMethod =
                dialog.authType() == ConnectDialog::AuthType::Password
                    ? SshAuthMethod::Password
                    : SshAuthMethod::PrivateKey;

            statusBar()->showMessage("Testing SSH authentication with " + tabTitle + "...");
            QApplication::setOverrideCursor(Qt::WaitCursor);
            QApplication::processEvents();

            authResult = SshSession::testAuthentication(
                dialog.host(),
                dialog.port(),
                dialog.username(),
                authMethod,
                dialog.password(),
                dialog.keyPath()
            );

            QApplication::restoreOverrideCursor();

            authAttempted = true;
            authSuccessful = authResult.success;

            output += QStringLiteral("Authentication result:\n");

            if (authResult.success) {
                output += QStringLiteral("Status: SUCCESS\n");
                output += QStringLiteral("Message: ") + authResult.message + QStringLiteral("\n");
                output += QStringLiteral("Auth return code: ") + QString::number(authResult.authReturnCode) + QStringLiteral("\n\n");

                output += QStringLiteral("Authentication works. Shell was NOT opened yet.\n\n");
                output += QStringLiteral("Next milestone:\n");
                output += QStringLiteral("- save successful connection to JSON\n");
                output += QStringLiteral("- persistent SSH session object\n");
                output += QStringLiteral("- real terminal channel\n");
            } else {
                output += QStringLiteral("Status: FAILED\n");
                output += QStringLiteral("Message: ") + authResult.message + QStringLiteral("\n");
                output += QStringLiteral("Auth return code: ") + QString::number(authResult.authReturnCode) + QStringLiteral("\n");
                output += QStringLiteral("libssh error code: ") + QString::number(authResult.sshErrorCode) + QStringLiteral("\n");
                output += QStringLiteral("Error: ") + authResult.error + QStringLiteral("\n\n");

                output += QStringLiteral("Check:\n");
                output += QStringLiteral("- username is correct\n");
                output += QStringLiteral("- password/private key is correct\n");
                output += QStringLiteral("- server allows selected authentication method\n");
                output += QStringLiteral("- private key has no passphrase for this early test\n");
            }
        } else {
            output += QStringLiteral("Connection flow stopped at known-host decision.\n");
            output += QStringLiteral("Authentication was NOT attempted.\n");
        }
    } else {
        output += QStringLiteral("Status: FAILED\n");
        output += QStringLiteral("Message: ") + handshake.message + QStringLiteral("\n");
        output += QStringLiteral("libssh error code: ") + QString::number(handshake.sshErrorCode) + QStringLiteral("\n");
        output += QStringLiteral("Error: ") + handshake.error + QStringLiteral("\n\n");
        output += QStringLiteral("Known-host result:\n");
        output += QStringLiteral("Decision: Not checked because handshake failed\n\n");
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

    if (handshake.success && knownHostAllowed && authAttempted && authSuccessful) {
        statusBar()->showMessage("SSH authentication successful for " + tabTitle);
    } else if (handshake.success && knownHostAllowed && authAttempted) {
        statusBar()->showMessage("SSH authentication failed for " + tabTitle);
    } else if (handshake.success && knownHostAllowed) {
        statusBar()->showMessage("SSH handshake and known-host check successful for " + tabTitle);
    } else if (handshake.success) {
        statusBar()->showMessage("SSH handshake successful, but known-host flow stopped for " + tabTitle);
    } else {
        statusBar()->showMessage("SSH handshake failed for " + tabTitle);
    }
}
