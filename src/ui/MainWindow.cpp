#include "MainWindow.h"
#include "ConnectDialog.h"
#include "BasicTerminalTab.h"
#include "WebTerminalTab.h"
#include "core/ConfigManager.h"
#include "core/KnownHostsManager.h"
#include "core/SessionProfile.h"
#include "ssh/SshSession.h"

#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <QFileDevice>
#include <QFontDatabase>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenuBar>
#include <QMessageBox>
#include <QMenu>
#include <QDir>
#include <QPushButton>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>
#include <QTemporaryFile>
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
            + QStringLiteral("Current phase: Web terminal direct input/paste dispatch fix.\n\n")
            + QStringLiteral("Version: ")
            + QCoreApplication::applicationVersion()
            + QStringLiteral("\n\n")
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

    loadSavedSessionsToSidebar();

    m_sessionList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_sessionList, &QListWidget::customContextMenuRequested, this, [this](const QPoint &position) {
        showSessionContextMenu(position);
    });

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
        const QString sessionId = item->data(Qt::UserRole).toString();

        if (sessionId.trimmed().isEmpty()) {
            return;
        }

        testSavedSession(sessionId);
    });

    splitter->addWidget(m_sessionList);
    splitter->addWidget(m_tabs);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    setCentralWidget(splitter);

    addWelcomeTab();
}

void MainWindow::loadSavedSessionsToSidebar()
{
    if (m_sessionList == nullptr) {
        return;
    }

    m_sessionList->clear();

    ConfigManager config;
    QString loadError;
    const QList<SessionProfile> sessions = config.loadSessions(&loadError);

    if (!loadError.isEmpty()) {
        auto *item = new QListWidgetItem("Could not load sessions");
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        m_sessionList->addItem(item);
        statusBar()->showMessage("Could not load saved sessions: " + loadError);
        return;
    }

    if (sessions.isEmpty()) {
        auto *item = new QListWidgetItem("No saved sessions yet");
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        m_sessionList->addItem(item);
        return;
    }

    for (const SessionProfile &session : sessions) {
        const QString label = session.group.trimmed().isEmpty()
            ? session.name
            : session.group.trimmed() + QStringLiteral(" / ") + session.name;

        auto *item = new QListWidgetItem(label);
        item->setData(Qt::UserRole, session.id);
        item->setToolTip(
            session.username
            + QStringLiteral("@")
            + session.host
            + QStringLiteral(":")
            + QString::number(session.port)
        );

        m_sessionList->addItem(item);
    }
}

void MainWindow::addWelcomeTab()
{
    auto *welcome = new QTextEdit(this);
    welcome->setReadOnly(true);
    welcome->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    welcome->setPlainText(
        "DD-SSH\n\n"
        "UI layout skeleton is alive.\n\n"
        "Left side: saved sessions from dd-ssh.json\n"
        "Right side: terminal tabs placeholder\n\n"
        "Double-click a saved session on the left to run the saved authentication test.\n\n"
        "Current milestone:\n"
        "- save successful connection to JSON\n"
        "- connect from saved session\n"
        "- config/session safety cleanup\n"
        "- delete saved sessions from sidebar context menu\n"
        "- edit/modify saved sessions from sidebar context menu\n"
        "- duplicate host/user warning polish for manual saves\n"
        "- basic saved-session SSH shell channel\n\n"
        "Current terminal options:\n"
        "- Open basic shell: temporary QWidget input/output view\n"
        "- Open web terminal: keyboard input directly inside terminal area, xterm.js-ready fallback renderer\n\n"
        "Next milestone:\n"
        "- bundle real xterm.js renderer\n"
        "- stronger persistent session lifecycle handling\n"
    );

    m_tabs->addTab(welcome, "Welcome");
}


void MainWindow::showSessionContextMenu(const QPoint &position)
{
    if (m_sessionList == nullptr) {
        return;
    }

    QListWidgetItem *item = m_sessionList->itemAt(position);

    if (item == nullptr) {
        return;
    }

    const QString sessionId = item->data(Qt::UserRole).toString();

    if (sessionId.trimmed().isEmpty()) {
        return;
    }

    QMenu menu(this);
    QAction *connectAction = menu.addAction("Connect / auth test");
    QAction *openWebTerminalAction = menu.addAction("Open web terminal (xterm-ready)");
    QAction *openShellAction = menu.addAction("Open basic shell (fallback)");
    menu.addSeparator();
    QAction *editAction = menu.addAction("Edit session");
    QAction *deleteAction = menu.addAction("Delete session");

    QAction *selectedAction = menu.exec(m_sessionList->viewport()->mapToGlobal(position));

    if (selectedAction == connectAction) {
        testSavedSession(sessionId);
    } else if (selectedAction == openWebTerminalAction) {
        openSavedSessionWebTerminal(sessionId);
    } else if (selectedAction == openShellAction) {
        openSavedSessionShell(sessionId);
    } else if (selectedAction == editAction) {
        editSavedSession(sessionId);
    } else if (selectedAction == deleteAction) {
        deleteSavedSession(sessionId);
    }
}

void MainWindow::editSavedSession(const QString &sessionId)
{
    ConfigManager config;
    SessionProfile existingSession;
    QString loadError;

    if (!config.loadSessionById(sessionId, &existingSession, &loadError)) {
        QMessageBox::warning(
            this,
            "Could not load saved session",
            loadError
        );
        statusBar()->showMessage("Could not load saved session for editing: " + sessionId);
        loadSavedSessionsToSidebar();
        return;
    }

    ConnectDialog dialog(this);
    dialog.setEditMode(true);
    dialog.setConnectionFields(
        existingSession.host,
        existingSession.port,
        existingSession.username,
        existingSession.authType == SessionProfile::AuthType::PrivateKey
            ? ConnectDialog::AuthType::PrivateKey
            : ConnectDialog::AuthType::Password
    );
    dialog.setSessionFields(existingSession.name, existingSession.group);

    if (dialog.exec() != QDialog::Accepted) {
        statusBar()->showMessage("Edit session cancelled");
        return;
    }

    SessionProfile updatedSession;
    updatedSession.name = dialog.sessionName();
    updatedSession.group = dialog.groupName();
    updatedSession.host = dialog.host();
    updatedSession.port = dialog.port();
    updatedSession.username = dialog.username();
    updatedSession.id = ConfigManager::makeSessionId(
        updatedSession.name,
        updatedSession.host,
        updatedSession.port,
        updatedSession.username
    );
    updatedSession.authType = dialog.authType() == ConnectDialog::AuthType::PrivateKey
        ? SessionProfile::AuthType::PrivateKey
        : SessionProfile::AuthType::Password;

    QString replacementSecret;
    bool replaceSecret = false;

    if (updatedSession.authType == SessionProfile::AuthType::Password) {
        if (!dialog.password().isEmpty()) {
            replacementSecret = dialog.password();
            replaceSecret = true;
        }
    } else {
        if (!dialog.keyPath().trimmed().isEmpty()) {
            QString expandedKeyPath = dialog.keyPath();

            if (expandedKeyPath.startsWith(QStringLiteral("~/"))) {
                expandedKeyPath = QDir::homePath() + expandedKeyPath.mid(1);
            }

            QFile keyFile(expandedKeyPath);

            if (!keyFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QMessageBox::warning(
                    this,
                    "Could not read private key",
                    "Could not read private key file for JSON save: " + keyFile.errorString()
                );
                statusBar()->showMessage("Could not read replacement private key");
                return;
            }

            replacementSecret = QString::fromUtf8(keyFile.readAll());
            keyFile.close();
            replaceSecret = true;
        }
    }

    const bool authTypeChanged = updatedSession.authType != existingSession.authType;

    if (authTypeChanged && !replaceSecret) {
        QMessageBox::warning(
            this,
            "New secret required",
            "Authentication type was changed. Please enter a new password or choose a new private key file."
        );
        statusBar()->showMessage("Edit session requires a new secret when auth type changes");
        return;
    }

    QString updateError;
    bool changedSessionId = false;

    if (!config.updateSessionWithOptionalPlainSecret(
            existingSession.id,
            updatedSession,
            replacementSecret,
            replaceSecret,
            &updateError,
            &changedSessionId
        )) {
        QMessageBox::warning(
            this,
            "Could not update saved session",
            updateError
        );
        statusBar()->showMessage("Could not update saved session: " + existingSession.name);
        return;
    }

    loadSavedSessionsToSidebar();

    QString message = "Updated saved session: " + updatedSession.name;

    if (changedSessionId) {
        message += " (session id changed)";
    }

    if (replaceSecret) {
        message += " and replaced plaintext secret";
    } else {
        message += " and kept existing plaintext secret";
    }

    statusBar()->showMessage(message);
}

void MainWindow::deleteSavedSession(const QString &sessionId)
{
    ConfigManager config;
    SessionProfile session;
    QString loadError;

    if (!config.loadSessionById(sessionId, &session, &loadError)) {
        QMessageBox::warning(
            this,
            "Could not load saved session",
            loadError
        );
        statusBar()->showMessage("Could not load saved session for deletion: " + sessionId);
        loadSavedSessionsToSidebar();
        return;
    }

    const QString target =
        session.username
        + QStringLiteral("@")
        + session.host
        + QStringLiteral(":")
        + QString::number(session.port);

    QMessageBox confirmBox(this);
    confirmBox.setIcon(QMessageBox::Warning);
    confirmBox.setWindowTitle("Delete saved session");
    confirmBox.setText("Delete saved session?");
    confirmBox.setInformativeText(
        "Session: " + session.name + "\n"
        "Target: " + target + "\n\n"
        "This removes the saved session and its password/private-key secret if no other session uses it.\n"
        "Known-host trust records will NOT be deleted."
    );

    QAbstractButton *deleteButton = confirmBox.addButton("Delete session", QMessageBox::DestructiveRole);
    QAbstractButton *cancelButton = confirmBox.addButton("Cancel", QMessageBox::RejectRole);
    confirmBox.setDefaultButton(qobject_cast<QPushButton *>(cancelButton));
    confirmBox.exec();

    if (confirmBox.clickedButton() != deleteButton) {
        statusBar()->showMessage("Delete session cancelled");
        return;
    }

    QString deleteError;
    bool removedSecret = false;
    QString removedSecretId;

    if (!config.deleteSession(session.id, &deleteError, &removedSecret, &removedSecretId)) {
        QMessageBox::warning(
            this,
            "Could not delete saved session",
            deleteError
        );
        statusBar()->showMessage("Could not delete saved session: " + session.name);
        return;
    }

    loadSavedSessionsToSidebar();

    QString message = "Deleted saved session: " + session.name;

    if (removedSecret) {
        message += " and removed unused secret";
    } else {
        message += ". Secret was kept because it is shared or missing";
    }

    statusBar()->showMessage(message);
}


void MainWindow::openSavedSessionShell(const QString &sessionId)
{
    openSavedSessionShellInternal(sessionId, false);
}

void MainWindow::openSavedSessionWebTerminal(const QString &sessionId)
{
    openSavedSessionShellInternal(sessionId, true);
}

void MainWindow::openSavedSessionShellInternal(const QString &sessionId, bool useWebTerminal)
{
    ConfigManager config;
    SessionProfile session;
    QString loadError;

    if (!config.loadSessionById(sessionId, &session, &loadError)) {
        QMessageBox::warning(
            this,
            "Could not load saved session",
            loadError
        );
        statusBar()->showMessage("Could not load saved session for shell: " + sessionId);
        return;
    }

    const QString tabTitle =
        session.username
        + QStringLiteral("@")
        + session.host
        + QStringLiteral(":")
        + QString::number(session.port);

    const QString secretRef = session.authType == SessionProfile::AuthType::Password
        ? session.secretRef
        : session.keyRef;

    QString secretValue;
    QString secretType;
    QString secretError;

    if (!config.loadPlainSecret(secretRef, &secretValue, &secretType, &secretError)) {
        QMessageBox::warning(
            this,
            "Could not load saved secret",
            secretError
        );
        statusBar()->showMessage("Could not load plaintext secret for shell: " + tabTitle);
        return;
    }

    const QString expectedSecretType = session.authType == SessionProfile::AuthType::Password
        ? QStringLiteral("password")
        : QStringLiteral("private_key");

    if (secretType != expectedSecretType) {
        QMessageBox::warning(
            this,
            "Saved secret type mismatch",
            "Expected secret type: " + expectedSecretType + "\n"
            "Actual secret type: " + secretType
        );
        statusBar()->showMessage("Saved secret type mismatch for shell: " + tabTitle);
        return;
    }

    statusBar()->showMessage("Checking known-host trust before opening shell for " + tabTitle + "...");
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();

    const SshHandshakeResult handshake = SshSession::testHandshake(
        session.host,
        session.port,
        session.username
    );

    QApplication::restoreOverrideCursor();

    if (!handshake.success) {
        QMessageBox::warning(
            this,
            "SSH handshake failed",
            "Could not connect before opening shell.\n\n"
            "Message: " + handshake.message + "\n"
            "Error: " + handshake.error
        );
        statusBar()->showMessage("Shell handshake failed for " + tabTitle);
        return;
    }

    KnownHostsManager knownHosts;
    const KnownHostsManager::CheckResult check = knownHosts.checkHost(
        session.host,
        session.port,
        handshake.hostKeyType,
        handshake.hostKeyFingerprint
    );

    bool knownHostAllowed = false;

    if (check.status == KnownHostsManager::HostStatus::Trusted) {
        knownHostAllowed = true;

        QString saveError;
        knownHosts.trustHost(
            session.host,
            session.port,
            handshake.hostKeyType,
            handshake.hostKeyFingerprint,
            &saveError
        );

        if (!saveError.isEmpty()) {
            statusBar()->showMessage("Shell known-host trusted, but last_seen update warning: " + saveError);
        }
    } else if (check.status == KnownHostsManager::HostStatus::Unknown) {
        QMessageBox messageBox(this);
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setWindowTitle("Unknown SSH host");
        messageBox.setText("This SSH host is not trusted yet.");
        messageBox.setInformativeText(
            "Host: " + session.host + ":" + QString::number(session.port) + "\n"
            "Key type: " + handshake.hostKeyType + "\n"
            "Fingerprint: " + handshake.hostKeyFingerprint + "\n\n"
            "Do you want to trust this host before opening a shell?"
        );

        QAbstractButton *trustOnceButton = messageBox.addButton("Trust once", QMessageBox::AcceptRole);
        QAbstractButton *trustPermanentlyButton = messageBox.addButton("Trust permanently", QMessageBox::AcceptRole);
        QAbstractButton *cancelButton = messageBox.addButton("Cancel", QMessageBox::RejectRole);

        messageBox.setDefaultButton(qobject_cast<QPushButton *>(cancelButton));
        messageBox.exec();

        if (messageBox.clickedButton() == trustOnceButton) {
            knownHostAllowed = true;
        } else if (messageBox.clickedButton() == trustPermanentlyButton) {
            QString saveError;

            if (knownHosts.trustHost(
                    session.host,
                    session.port,
                    handshake.hostKeyType,
                    handshake.hostKeyFingerprint,
                    &saveError
                )) {
                knownHostAllowed = true;
            } else {
                QMessageBox::warning(
                    this,
                    "Could not save known host",
                    saveError
                );
            }
        }
    } else {
        QMessageBox::critical(
            this,
            "SSH host key changed",
            "WARNING: the SSH host key changed.\n\n"
            "Host: " + session.host + ":" + QString::number(session.port) + "\n\n"
            "Stored key type: " + check.storedKeyType + "\n"
            "Stored fingerprint: " + check.storedFingerprint + "\n\n"
            "Current key type: " + handshake.hostKeyType + "\n"
            "Current fingerprint: " + handshake.hostKeyFingerprint + "\n\n"
            "Shell was NOT opened."
        );
    }

    if (!knownHostAllowed) {
        statusBar()->showMessage("Shell open cancelled by known-host decision for " + tabTitle);
        return;
    }

    QWidget *terminal = nullptr;
    QString titleSuffix;

    if (useWebTerminal) {
        terminal = new WebTerminalTab(session, secretValue, this);
        titleSuffix = QStringLiteral(" web");
    } else {
        terminal = new BasicTerminalTab(session, secretValue, this);
        titleSuffix = QStringLiteral(" shell");
    }

    const int tabIndex = m_tabs->addTab(terminal, session.name + titleSuffix);
    m_tabs->setCurrentIndex(tabIndex);

    statusBar()->showMessage(
        useWebTerminal
            ? "Opening web terminal for " + tabTitle
            : "Opening basic shell for " + tabTitle
    );
}

void MainWindow::testSavedSession(const QString &sessionId)
{
    ConfigManager config;
    SessionProfile session;
    QString loadError;

    if (!config.loadSessionById(sessionId, &session, &loadError)) {
        QMessageBox::warning(
            this,
            "Could not load saved session",
            loadError
        );
        statusBar()->showMessage("Could not load saved session: " + sessionId);
        return;
    }

    const QString tabTitle =
        session.username
        + QStringLiteral("@")
        + session.host
        + QStringLiteral(":")
        + QString::number(session.port);

    const QString authLabel =
        session.authType == SessionProfile::AuthType::Password
            ? QStringLiteral("Password")
            : QStringLiteral("Private key");

    const QString secretRef =
        session.authType == SessionProfile::AuthType::Password
            ? session.secretRef
            : session.keyRef;

    QString secretValue;
    QString secretType;
    QString secretError;
    const bool secretLoaded = config.loadPlainSecret(
        secretRef,
        &secretValue,
        &secretType,
        &secretError
    );

    QString output;
    output += QStringLiteral("DD-SSH saved session connection test\n\n");
    output += QStringLiteral("Session: ") + session.name + QStringLiteral("\n");
    output += QStringLiteral("Session id: ") + session.id + QStringLiteral("\n");
    output += QStringLiteral("Group: ") + (session.group.trimmed().isEmpty() ? QStringLiteral("(none)") : session.group) + QStringLiteral("\n\n");
    output += QStringLiteral("Host: ") + session.host + QStringLiteral("\n");
    output += QStringLiteral("Port: ") + QString::number(session.port) + QStringLiteral("\n");
    output += QStringLiteral("Username: ") + session.username + QStringLiteral("\n");
    output += QStringLiteral("Auth type: ") + authLabel + QStringLiteral("\n");
    output += QStringLiteral("Secret ref: ") + secretRef + QStringLiteral("\n");

    if (session.authType == SessionProfile::AuthType::Password) {
        output += QStringLiteral("Saved password: loaded from JSON, hidden from display\n\n");
    } else {
        output += QStringLiteral("Saved private key: loaded from JSON, hidden from display\n\n");
    }

    if (!secretLoaded) {
        output += QStringLiteral("Secret load result:\n");
        output += QStringLiteral("Status: FAILED\n");
        output += QStringLiteral("Message: ") + secretError + QStringLiteral("\n\n");
        output += QStringLiteral("Authentication was NOT attempted.\n");

        auto *terminalPlaceholder = new QTextEdit(this);
        terminalPlaceholder->setReadOnly(true);
        terminalPlaceholder->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        terminalPlaceholder->setPlainText(output);

        const int tabIndex = m_tabs->addTab(terminalPlaceholder, session.name);
        m_tabs->setCurrentIndex(tabIndex);
        statusBar()->showMessage("Saved session secret could not be loaded for " + tabTitle);
        return;
    }

    const QString expectedSecretType =
        session.authType == SessionProfile::AuthType::Password
            ? QStringLiteral("password")
            : QStringLiteral("private_key");

    if (secretType != expectedSecretType) {
        output += QStringLiteral("Secret load result:\n");
        output += QStringLiteral("Status: FAILED\n");
        output += QStringLiteral("Message: Secret type mismatch. Expected ")
            + expectedSecretType
            + QStringLiteral(", got ")
            + secretType
            + QStringLiteral(".\n\n");
        output += QStringLiteral("Authentication was NOT attempted.\n");

        auto *terminalPlaceholder = new QTextEdit(this);
        terminalPlaceholder->setReadOnly(true);
        terminalPlaceholder->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        terminalPlaceholder->setPlainText(output);

        const int tabIndex = m_tabs->addTab(terminalPlaceholder, session.name);
        m_tabs->setCurrentIndex(tabIndex);
        statusBar()->showMessage("Saved session secret type mismatch for " + tabTitle);
        return;
    }

    output += QStringLiteral("Secret load result:\n");
    output += QStringLiteral("Status: SUCCESS\n");
    output += QStringLiteral("Message: Plaintext secret loaded from dd-ssh.json. Value is hidden.\n\n");

    statusBar()->showMessage("Testing saved SSH session with " + tabTitle + "...");
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();

    const SshHandshakeResult handshake = SshSession::testHandshake(
        session.host,
        session.port,
        session.username
    );

    QApplication::restoreOverrideCursor();

    KnownHostsManager knownHosts;

    bool knownHostAllowed = false;
    QString knownHostDecision = QStringLiteral("Not checked");
    QString knownHostExtra;

    if (handshake.success) {
        const KnownHostsManager::CheckResult check = knownHosts.checkHost(
            session.host,
            session.port,
            handshake.hostKeyType,
            handshake.hostKeyFingerprint
        );

        if (check.status == KnownHostsManager::HostStatus::Trusted) {
            knownHostAllowed = true;
            knownHostDecision = QStringLiteral("TRUSTED - stored fingerprint matches current host key");

            QString saveError;
            knownHosts.trustHost(
                session.host,
                session.port,
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
                "Host: " + session.host + ":" + QString::number(session.port) + "\n"
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
                        session.host,
                        session.port,
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
                "Host: " + session.host + ":" + QString::number(session.port) + "\n\n"
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
                        session.host,
                        session.port,
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

    output += QStringLiteral("SSH handshake result:\n");

    bool authAttempted = false;
    bool authSuccessful = false;
    SshAuthResult authResult;
    QString tempKeyError;

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
                session.authType == SessionProfile::AuthType::Password
                    ? SshAuthMethod::Password
                    : SshAuthMethod::PrivateKey;

            QString passwordForAuth;
            QString keyPathForAuth;
            QTemporaryFile tempKeyFile(QDir::tempPath() + QStringLiteral("/dd-ssh-key-XXXXXX"));

            if (session.authType == SessionProfile::AuthType::Password) {
                passwordForAuth = secretValue;
            } else {
                tempKeyFile.setAutoRemove(true);

                if (!tempKeyFile.open()) {
                    tempKeyError = QStringLiteral("Could not create temporary private key file: ") + tempKeyFile.errorString();
                } else {
                    tempKeyFile.write(secretValue.toUtf8());
                    tempKeyFile.flush();
                    keyPathForAuth = tempKeyFile.fileName();
                    tempKeyFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
                    tempKeyFile.close();
                }
            }

            if (!tempKeyError.isEmpty()) {
                output += QStringLiteral("Authentication result:\n");
                output += QStringLiteral("Status: FAILED\n");
                output += QStringLiteral("Message: ") + tempKeyError + QStringLiteral("\n");
            } else {
                statusBar()->showMessage("Testing saved SSH authentication with " + tabTitle + "...");
                QApplication::setOverrideCursor(Qt::WaitCursor);
                QApplication::processEvents();

                authResult = SshSession::testAuthentication(
                    session.host,
                    session.port,
                    session.username,
                    authMethod,
                    passwordForAuth,
                    keyPathForAuth
                );

                QApplication::restoreOverrideCursor();

                authAttempted = true;
                authSuccessful = authResult.success;

                output += QStringLiteral("Authentication result:\n");

                if (authResult.success) {
                    output += QStringLiteral("Status: SUCCESS\n");
                    output += QStringLiteral("Message: ") + authResult.message + QStringLiteral("\n");
                    output += QStringLiteral("Auth return code: ") + QString::number(authResult.authReturnCode) + QStringLiteral("\n\n");
                    output += QStringLiteral("Saved session authentication works. Shell was NOT opened yet.\n\n");
                    output += QStringLiteral("Next milestone:\n");
                    output += QStringLiteral("- persistent SSH session object\n");
                    output += QStringLiteral("- real terminal channel\n");
                } else {
                    output += QStringLiteral("Status: FAILED\n");
                    output += QStringLiteral("Message: ") + authResult.message + QStringLiteral("\n");
                    output += QStringLiteral("Auth return code: ") + QString::number(authResult.authReturnCode) + QStringLiteral("\n");
                    output += QStringLiteral("libssh error code: ") + QString::number(authResult.sshErrorCode) + QStringLiteral("\n");
                    output += QStringLiteral("Error: ") + authResult.error + QStringLiteral("\n\n");
                    output += QStringLiteral("Check:\n");
                    output += QStringLiteral("- saved password/private key is correct\n");
                    output += QStringLiteral("- server still allows selected authentication method\n");
                    output += QStringLiteral("- private key has no passphrase for this early test\n");
                }
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

    const int tabIndex = m_tabs->addTab(terminalPlaceholder, session.name);
    m_tabs->setCurrentIndex(tabIndex);

    if (handshake.success && knownHostAllowed && authAttempted && authSuccessful) {
        statusBar()->showMessage("Saved session authentication successful for " + tabTitle);
    } else if (handshake.success && knownHostAllowed && authAttempted) {
        statusBar()->showMessage("Saved session authentication failed for " + tabTitle);
    } else if (handshake.success && knownHostAllowed) {
        statusBar()->showMessage("Saved session handshake and known-host check successful for " + tabTitle);
    } else if (handshake.success) {
        statusBar()->showMessage("Saved session handshake successful, but known-host flow stopped for " + tabTitle);
    } else {
        statusBar()->showMessage("Saved session handshake failed for " + tabTitle);
    }
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
    bool sessionSaveAttempted = false;
    bool sessionSaved = false;
    bool sessionUpdatedExisting = false;
    bool sessionSaveCancelled = false;
    bool sessionSavedAsCopy = false;
    QString sessionSaveMessage;
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

                if (dialog.saveConnection()) {
                    sessionSaveAttempted = true;

                    ConfigManager config;
                    SessionProfile session;
                    session.name = dialog.sessionName();
                    session.group = dialog.groupName();
                    session.host = dialog.host();
                    session.port = dialog.port();
                    session.username = dialog.username();
                    session.id = ConfigManager::makeSessionId(
                        session.name,
                        session.host,
                        session.port,
                        session.username
                    );

                    QString secretValue;

                    if (dialog.authType() == ConnectDialog::AuthType::PrivateKey) {
                        session.authType = SessionProfile::AuthType::PrivateKey;

                        QString expandedKeyPath = dialog.keyPath();

                        if (expandedKeyPath.startsWith(QStringLiteral("~/"))) {
                            expandedKeyPath = QDir::homePath() + expandedKeyPath.mid(1);
                        }

                        QFile keyFile(expandedKeyPath);

                        if (keyFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                            secretValue = QString::fromUtf8(keyFile.readAll());
                            keyFile.close();
                        } else {
                            sessionSaveMessage = QStringLiteral("Could not read private key file for JSON save: ") + keyFile.errorString();
                        }
                    } else {
                        session.authType = SessionProfile::AuthType::Password;
                        secretValue = dialog.password();
                    }

                    if (sessionSaveMessage.isEmpty()) {
                        QString duplicateCheckError;
                        const QList<SessionProfile> duplicateTargetSessions = config.findSessionsByTarget(
                            session.host,
                            session.port,
                            session.username,
                            QString(),
                            &duplicateCheckError
                        );

                        if (!duplicateCheckError.isEmpty()) {
                            sessionSaveMessage = QStringLiteral("Could not check for duplicate saved sessions: ")
                                + duplicateCheckError;
                        } else if (!duplicateTargetSessions.isEmpty()) {
                            const SessionProfile existingSession = duplicateTargetSessions.first();
                            const QString target =
                                session.username
                                + QStringLiteral("@")
                                + session.host
                                + QStringLiteral(":")
                                + QString::number(session.port);

                            QMessageBox duplicateBox(this);
                            duplicateBox.setIcon(QMessageBox::Question);
                            duplicateBox.setWindowTitle("Saved session already exists");
                            duplicateBox.setText("A session for " + target + " already exists.");

                            QString duplicateInfo =
                                QStringLiteral("Existing session: ") + existingSession.name + QStringLiteral("\n")
                                + QStringLiteral("Existing session id: ") + existingSession.id + QStringLiteral("\n")
                                + QStringLiteral("New session name: ") + session.name + QStringLiteral("\n\n")
                                + QStringLiteral("What do you want to do?");

                            if (duplicateTargetSessions.size() > 1) {
                                duplicateInfo +=
                                    QStringLiteral("\n\nNote: there are ")
                                    + QString::number(duplicateTargetSessions.size())
                                    + QStringLiteral(" saved sessions for this same target. Update existing will update the first match shown above.");
                            }

                            duplicateBox.setInformativeText(duplicateInfo);

                            QAbstractButton *updateExistingButton = duplicateBox.addButton(
                                "Update existing",
                                QMessageBox::AcceptRole
                            );
                            QAbstractButton *createCopyButton = duplicateBox.addButton(
                                "Create copy",
                                QMessageBox::ActionRole
                            );
                            QAbstractButton *cancelButton = duplicateBox.addButton(
                                "Cancel",
                                QMessageBox::RejectRole
                            );

                            duplicateBox.setDefaultButton(qobject_cast<QPushButton *>(cancelButton));
                            duplicateBox.exec();

                            if (duplicateBox.clickedButton() == updateExistingButton) {
                                QString updateError;
                                bool changedSessionId = false;

                                sessionSaved = config.updateSessionWithOptionalPlainSecret(
                                    existingSession.id,
                                    session,
                                    secretValue,
                                    true,
                                    &updateError,
                                    &changedSessionId
                                );

                                sessionUpdatedExisting = sessionSaved;

                                if (sessionSaved) {
                                    sessionSaveMessage = changedSessionId
                                        ? QStringLiteral("Updated existing session and changed its session id in ") + config.configFilePath()
                                        : QStringLiteral("Updated existing session in ") + config.configFilePath();
                                    loadSavedSessionsToSidebar();
                                } else {
                                    sessionSaveMessage = updateError;
                                }
                            } else if (duplicateBox.clickedButton() == createCopyButton) {
                                SessionProfile copySession = session;
                                QString uniqueIdError;
                                QString uniqueSessionId = config.makeUniqueSessionId(
                                    copySession.id,
                                    QString(),
                                    &uniqueIdError
                                );

                                if (!uniqueIdError.isEmpty()) {
                                    sessionSaveMessage = QStringLiteral("Could not prepare unique session id: ")
                                        + uniqueIdError;
                                } else {
                                    if (uniqueSessionId != copySession.id) {
                                        copySession.name = copySession.name.trimmed() + QStringLiteral(" (copy)");
                                        copySession.id = ConfigManager::makeSessionId(
                                            copySession.name,
                                            copySession.host,
                                            copySession.port,
                                            copySession.username
                                        );
                                        uniqueSessionId = config.makeUniqueSessionId(
                                            copySession.id,
                                            QString(),
                                            &uniqueIdError
                                        );

                                        if (!uniqueIdError.isEmpty()) {
                                            sessionSaveMessage = QStringLiteral("Could not prepare copy session id: ")
                                                + uniqueIdError;
                                        } else {
                                            copySession.id = uniqueSessionId;
                                        }
                                    } else {
                                        copySession.id = uniqueSessionId;
                                    }

                                    if (sessionSaveMessage.isEmpty()) {
                                        QString saveError;

                                        sessionSaved = config.saveSessionWithPlainSecret(
                                            copySession,
                                            secretValue,
                                            &saveError,
                                            &sessionUpdatedExisting
                                        );

                                        sessionSavedAsCopy = sessionSaved;

                                        if (sessionSaved) {
                                            sessionSaveMessage = QStringLiteral("Created copy as new session '")
                                                + copySession.name
                                                + QStringLiteral("' in ")
                                                + config.configFilePath();
                                            loadSavedSessionsToSidebar();
                                        } else {
                                            sessionSaveMessage = saveError;
                                        }
                                    }
                                }
                            } else {
                                sessionSaveCancelled = true;
                                sessionSaveMessage = QStringLiteral("User cancelled saving because a matching saved session already exists.");
                            }
                        } else {
                            QString saveError;

                            sessionSaved = config.saveSessionWithPlainSecret(
                                session,
                                secretValue,
                                &saveError,
                                &sessionUpdatedExisting
                            );

                            if (sessionSaved) {
                                sessionSaveMessage = sessionUpdatedExisting
                                    ? QStringLiteral("Updated existing session in ") + config.configFilePath()
                                    : QStringLiteral("Created new session in ") + config.configFilePath();
                                loadSavedSessionsToSidebar();
                            } else {
                                sessionSaveMessage = saveError;
                            }
                        }
                    }
                }

                if (sessionSaveAttempted) {
                    output += QStringLiteral("Session save result:\n");

                    if (sessionSaveCancelled) {
                        output += QStringLiteral("Status: CANCELLED\n");
                    } else if (sessionSavedAsCopy) {
                        output += QStringLiteral("Status: CREATED COPY\n");
                    } else if (sessionSaved) {
                        output += sessionUpdatedExisting
                            ? QStringLiteral("Status: UPDATED EXISTING SESSION\n")
                            : QStringLiteral("Status: CREATED NEW SESSION\n");
                    } else {
                        output += QStringLiteral("Status: FAILED\n");
                    }

                    output += QStringLiteral("Message: ") + sessionSaveMessage + QStringLiteral("\n");
                    output += QStringLiteral("Secrets mode: plain-v1 plaintext portable storage\n\n");
                } else {
                    output += QStringLiteral("Session save result:\n");
                    output += QStringLiteral("Status: SKIPPED - Save this connection was not checked.\n\n");
                }

                output += QStringLiteral("Authentication works. Shell was NOT opened yet.\n\n");
                output += QStringLiteral("Next milestone:\n");
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
        if (sessionSaveAttempted && sessionSaveCancelled) {
            statusBar()->showMessage("SSH authentication successful; session save cancelled for " + tabTitle);
        } else if (sessionSaveAttempted && sessionSaved) {
            if (sessionSavedAsCopy) {
                statusBar()->showMessage("SSH authentication successful and session copy created for " + tabTitle);
            } else {
                statusBar()->showMessage(
                    sessionUpdatedExisting
                        ? QStringLiteral("SSH authentication successful and session updated for ") + tabTitle
                        : QStringLiteral("SSH authentication successful and session created for ") + tabTitle
                );
            }
        } else if (sessionSaveAttempted) {
            statusBar()->showMessage("SSH authentication successful, but session save failed for " + tabTitle);
        } else {
            statusBar()->showMessage("SSH authentication successful for " + tabTitle);
        }
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
