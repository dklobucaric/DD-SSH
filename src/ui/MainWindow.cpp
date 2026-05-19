#include "MainWindow.h"
#include "ConnectDialog.h"
#include "BasicTerminalTab.h"
#include "WebTerminalTab.h"
#include "SettingsDialog.h"
#include "core/ConfigManager.h"
#include "core/KnownHostsManager.h"
#include "core/SessionProfile.h"
#include "ssh/SshSession.h"

#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QFile>
#include <QFileDevice>
#include <QFileDialog>
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
#include <QUrl>

#ifndef DD_SSH_CODENAME_STRING
#define DD_SSH_CODENAME_STRING "unnamed"
#endif

#ifndef DD_SSH_MILESTONE_STRING
#define DD_SSH_MILESTONE_STRING "development"
#endif


namespace {
QString normalizedAppTheme(const QString &themeName)
{
    const QString normalized = themeName.trimmed().toLower();

    if (normalized == QStringLiteral("light") || normalized == QStringLiteral("dark")) {
        return normalized;
    }

    return QStringLiteral("system");
}

QString lightAppStyleSheet()
{
    return QStringLiteral(R"DDSSH(
QMainWindow,
QDialog,
QMessageBox,
QWidget {
    background-color: #f6f7f9;
    color: #202124;
}
QMenuBar,
QMenu,
QToolBar,
QStatusBar {
    background-color: #ffffff;
    color: #202124;
}
QMenuBar::item:selected,
QMenu::item:selected {
    background-color: #e8f0fe;
}
QSplitter::handle {
    background-color: #d0d5dd;
}
QGroupBox {
    border: 1px solid #cfd4dc;
    border-radius: 4px;
    margin-top: 8px;
    padding-top: 12px;
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 8px;
    padding: 0 4px;
}
QLineEdit,
QSpinBox,
QComboBox,
QListWidget,
QTextEdit,
QPlainTextEdit {
    background-color: #ffffff;
    color: #202124;
    border: 1px solid #b8c0cc;
    selection-background-color: #cfe3ff;
    selection-color: #111111;
}
QPushButton {
    background-color: #ffffff;
    color: #202124;
    border: 1px solid #aab2c0;
    border-radius: 4px;
    padding: 4px 10px;
}
QPushButton:hover {
    background-color: #eef4ff;
}
QPushButton:disabled {
    color: #8a8f98;
    background-color: #f1f3f5;
}
QTabWidget::pane {
    border: 1px solid #cfd4dc;
}
QTabBar::tab {
    background-color: #edf1f5;
    color: #202124;
    border: 1px solid #cfd4dc;
    padding: 5px 10px;
}
QTabBar::tab:selected {
    background-color: #ffffff;
}
)DDSSH");
}

QString darkAppStyleSheet()
{
    return QStringLiteral(R"DDSSH(
QMainWindow,
QDialog,
QMessageBox,
QWidget {
    background-color: #1f232a;
    color: #e6edf3;
}
QMenuBar,
QMenu,
QToolBar,
QStatusBar {
    background-color: #151922;
    color: #e6edf3;
}
QMenuBar::item:selected,
QMenu::item:selected {
    background-color: #263449;
}
QSplitter::handle {
    background-color: #303846;
}
QGroupBox {
    border: 1px solid #3a4354;
    border-radius: 4px;
    margin-top: 8px;
    padding-top: 12px;
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 8px;
    padding: 0 4px;
}
QLabel {
    color: #e6edf3;
}
QLineEdit,
QSpinBox,
QComboBox,
QListWidget,
QTextEdit,
QPlainTextEdit {
    background-color: #0f141b;
    color: #e6edf3;
    border: 1px solid #3a4354;
    selection-background-color: #315f9f;
    selection-color: #ffffff;
}
QListWidget::item:selected {
    background-color: #2f7d46;
    color: #ffffff;
}
QPushButton {
    background-color: #26303d;
    color: #e6edf3;
    border: 1px solid #536174;
    border-radius: 4px;
    padding: 4px 10px;
}
QPushButton:hover {
    background-color: #334155;
}
QPushButton:disabled {
    color: #7d8794;
    background-color: #202631;
}
QTabWidget::pane {
    border: 1px solid #3a4354;
}
QTabBar::tab {
    background-color: #202631;
    color: #d8dee9;
    border: 1px solid #3a4354;
    padding: 5px 10px;
}
QTabBar::tab:selected {
    background-color: #111827;
    color: #ffffff;
}
)DDSSH");
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("DD-SSH");

    ConfigManager config;
    QString settingsError;
    const AppSettings startupSettings = config.loadSettings(&settingsError);

    if (settingsError.isEmpty()) {
        applyAppTheme(startupSettings.appTheme);
    }

    setupMenus();
    setupToolbar();
    applyQuickToolbarVisibility(startupSettings.showQuickToolbar);
    setupCentralLayout();

    statusBar()->showMessage("DD-SSH Andromeda ready — public alpha release preparation");

    resize(1100, 700);
    showConfigRecoveryWarningIfNeeded();
}

void MainWindow::applyAppTheme(const QString &themeName)
{
    const QString normalized = normalizedAppTheme(themeName);

    if (normalized == QStringLiteral("dark")) {
        qApp->setStyleSheet(darkAppStyleSheet());
        return;
    }

    if (normalized == QStringLiteral("light")) {
        qApp->setStyleSheet(lightAppStyleSheet());
        return;
    }

    qApp->setStyleSheet(QString());
}

void MainWindow::applyQuickToolbarVisibility(bool showQuickToolbar)
{
    if (m_mainToolBar == nullptr) {
        return;
    }

    m_mainToolBar->setVisible(showQuickToolbar);
}

void MainWindow::openConfigFolder()
{
    ConfigManager config;
    QDir directory(config.configDirectoryPath());

    if (!directory.exists()) {
        directory.mkpath(QStringLiteral("."));
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(directory.absolutePath()));
}


void MainWindow::exportConfig()
{
    ConfigManager config;

    QString defaultFileName = QDir::home().filePath(QStringLiteral("dd-ssh-export.json"));
    const QString targetPath = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("Export DD-SSH config"),
        defaultFileName,
        QStringLiteral("JSON files (*.json);;All files (*)")
    );

    if (targetPath.trimmed().isEmpty()) {
        return;
    }

    QString errorMessage;

    if (!config.exportConfigToFile(targetPath, &errorMessage)) {
        QMessageBox::critical(
            this,
            QStringLiteral("Export config failed — DD-SSH"),
            errorMessage
        );
        statusBar()->showMessage(QStringLiteral("Config export failed"));
        return;
    }

    QMessageBox::information(
        this,
        QStringLiteral("Config exported — DD-SSH"),
        QStringLiteral("DD-SSH config was exported to:\n%1\n\nSecurity note: exported configs may contain plaintext passwords and private keys.")
            .arg(targetPath)
    );
    statusBar()->showMessage(QStringLiteral("Config exported to %1").arg(targetPath));
}

void MainWindow::importConfig()
{
    ConfigManager config;

    const QString sourcePath = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("Import DD-SSH config"),
        QDir::homePath(),
        QStringLiteral("JSON files (*.json);;All files (*)")
    );

    if (sourcePath.trimmed().isEmpty()) {
        return;
    }

    const QMessageBox::StandardButton decision = QMessageBox::warning(
        this,
        QStringLiteral("Import config — DD-SSH"),
        QStringLiteral(
            "Importing a config will replace the active dd-ssh.json.\n\n"
            "This includes saved sessions, plaintext secrets, known_hosts, settings, and metadata.\n\n"
            "DD-SSH will create a pre-import backup of the current config before replacing it.\n\n"
            "Import file:\n%1\n\n"
            "Continue?"
        ).arg(sourcePath),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel
    );

    if (decision != QMessageBox::Yes) {
        statusBar()->showMessage(QStringLiteral("Config import cancelled"));
        return;
    }

    QString errorMessage;
    QString backupPath;

    if (!config.importConfigFromFile(sourcePath, &errorMessage, &backupPath)) {
        QMessageBox::critical(
            this,
            QStringLiteral("Import config failed — DD-SSH"),
            errorMessage
        );
        statusBar()->showMessage(QStringLiteral("Config import failed"));
        return;
    }

    QString info = QStringLiteral("DD-SSH config was imported from:\n%1").arg(sourcePath);

    if (!backupPath.isEmpty()) {
        info += QStringLiteral("\n\nPrevious config backup created at:\n%1").arg(backupPath);
    }

    QMessageBox::information(
        this,
        QStringLiteral("Config imported — DD-SSH"),
        info
    );

    QString settingsError;
    const AppSettings settings = config.loadSettings(&settingsError);

    if (settingsError.isEmpty()) {
        applyAppTheme(settings.appTheme);
        applyQuickToolbarVisibility(settings.showQuickToolbar);
    }

    loadSavedSessionsToSidebar();
    statusBar()->showMessage(QStringLiteral("Config imported. Saved sessions and settings reloaded."));
}


void MainWindow::restoreLatestConfigBackup()
{
    ConfigManager config;

    const QMessageBox::StandardButton decision = QMessageBox::warning(
        this,
        QStringLiteral("Restore latest config backup — DD-SSH"),
        QStringLiteral(
            "This will replace the active dd-ssh.json with the newest valid dd-ssh.json.bak-* backup.\n\n"
            "The current config will be moved aside first as dd-ssh.json.pre-restore-<timestamp>.\n\n"
            "Continue?"
        ),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel
    );

    if (decision != QMessageBox::Yes) {
        statusBar()->showMessage(QStringLiteral("Config backup restore cancelled"));
        return;
    }

    QString errorMessage;
    QString restoredBackupName;
    QString movedCurrentPath;

    if (!config.restoreLatestBackupReplacingCurrent(&errorMessage, &restoredBackupName, &movedCurrentPath)) {
        QMessageBox::critical(
            this,
            QStringLiteral("Restore backup failed — DD-SSH"),
            errorMessage
        );
        statusBar()->showMessage(QStringLiteral("Config backup restore failed"));
        return;
    }

    QString info = QStringLiteral("Restored latest valid backup:\n%1").arg(restoredBackupName);

    if (!movedCurrentPath.isEmpty()) {
        info += QStringLiteral("\n\nPrevious active config moved to:\n%1").arg(movedCurrentPath);
    }

    QMessageBox::information(
        this,
        QStringLiteral("Backup restored — DD-SSH"),
        info
    );

    QString settingsError;
    const AppSettings settings = config.loadSettings(&settingsError);

    if (settingsError.isEmpty()) {
        applyAppTheme(settings.appTheme);
        applyQuickToolbarVisibility(settings.showQuickToolbar);
    }

    loadSavedSessionsToSidebar();
    statusBar()->showMessage(QStringLiteral("Latest config backup restored. Saved sessions and settings reloaded."));
}

void MainWindow::showConfigRecoveryWarningIfNeeded()
{
    ConfigManager config;
    const ConfigInspection inspection = config.inspectConfig();

    if (!inspection.hasProblem) {
        return;
    }

    showConfigRecoveryDialog(inspection);
}

bool MainWindow::showConfigRecoveryDialog(const ConfigInspection &inspection)
{
    QString details = inspection.message
        + QStringLiteral("\n\nConfig file:\n")
        + inspection.configFilePath
        + QStringLiteral("\n\nDD-SSH will not overwrite this file automatically. You can continue read-only, restore the latest valid backup, or move the corrupt file aside and create a fresh empty config.");

    if (!inspection.backupFileNames.isEmpty()) {
        details += QStringLiteral("\n\nAvailable backups in the config folder:");

        const int maxShownBackups = 10;

        for (int i = 0; i < inspection.backupFileNames.size() && i < maxShownBackups; ++i) {
            details += QStringLiteral("\n- ") + inspection.backupFileNames.at(i);
        }

        if (inspection.backupFileNames.size() > maxShownBackups) {
            details += QStringLiteral("\n- ...");
        }
    } else {
        details += QStringLiteral("\n\nNo dd-ssh.json.bak-* backup files were found in the config folder.");
    }

    QMessageBox messageBox(this);
    messageBox.setIcon(QMessageBox::Warning);
    messageBox.setWindowTitle(QStringLiteral("DD-SSH config recovery"));
    messageBox.setText(inspection.title.isEmpty()
        ? QStringLiteral("Config file problem detected")
        : inspection.title);
    messageBox.setInformativeText(details);

    QAbstractButton *openFolderButton = messageBox.addButton(
        QStringLiteral("Open config folder"),
        QMessageBox::ActionRole
    );
    QAbstractButton *restoreButton = messageBox.addButton(
        QStringLiteral("Restore latest valid backup"),
        QMessageBox::ActionRole
    );
    QAbstractButton *freshConfigButton = messageBox.addButton(
        QStringLiteral("Create fresh config"),
        QMessageBox::DestructiveRole
    );
    QAbstractButton *continueButton = messageBox.addButton(
        QStringLiteral("Continue read-only"),
        QMessageBox::AcceptRole
    );
    messageBox.setDefaultButton(qobject_cast<QPushButton *>(continueButton));
    messageBox.exec();

    ConfigManager config;

    if (messageBox.clickedButton() == openFolderButton) {
        openConfigFolder();
        statusBar()->showMessage(QStringLiteral("Opened config folder. Corrupt dd-ssh.json was not overwritten."));
        return false;
    }

    if (messageBox.clickedButton() == restoreButton) {
        QString error;
        QString restoredBackupName;
        QString movedCorruptPath;

        if (!config.restoreLatestValidBackup(&error, &restoredBackupName, &movedCorruptPath)) {
            QMessageBox::warning(
                this,
                QStringLiteral("Could not restore backup"),
                error.isEmpty()
                    ? QStringLiteral("DD-SSH could not restore the latest valid backup.")
                    : error
            );
            statusBar()->showMessage(QStringLiteral("Config recovery failed: backup restore did not complete"));
            return false;
        }

        loadSavedSessionsToSidebar();
        QString settingsError;
        const AppSettings restoredSettings = config.loadSettings(&settingsError);

        if (settingsError.isEmpty()) {
            applyAppTheme(restoredSettings.appTheme);
            applyQuickToolbarVisibility(restoredSettings.showQuickToolbar);
        }

        QMessageBox::information(
            this,
            QStringLiteral("Config backup restored"),
            QStringLiteral("Restored backup:\n%1\n\nCorrupt config moved to:\n%2")
                .arg(restoredBackupName.isEmpty() ? QStringLiteral("latest valid backup") : restoredBackupName)
                .arg(movedCorruptPath.isEmpty() ? QStringLiteral("(no previous config file was present)") : movedCorruptPath)
        );
        statusBar()->showMessage(QStringLiteral("Config restored from backup: ") + restoredBackupName);
        return true;
    }

    if (messageBox.clickedButton() == freshConfigButton) {
        const QMessageBox::StandardButton decision = QMessageBox::warning(
            this,
            QStringLiteral("Create fresh config?"),
            QStringLiteral("DD-SSH will move the corrupt dd-ssh.json aside and create a fresh empty config.\n\nSaved sessions, known_hosts, and plaintext secrets will not be copied into the new config. The old corrupt file will be preserved for manual recovery.\n\nContinue?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );

        if (decision != QMessageBox::Yes) {
            statusBar()->showMessage(QStringLiteral("Create fresh config cancelled"));
            return false;
        }

        QString error;
        QString movedCorruptPath;

        if (!config.createFreshConfigFromCorrupt(&error, &movedCorruptPath)) {
            QMessageBox::warning(
                this,
                QStringLiteral("Could not create fresh config"),
                error.isEmpty()
                    ? QStringLiteral("DD-SSH could not create a fresh config.")
                    : error
            );
            statusBar()->showMessage(QStringLiteral("Config recovery failed: fresh config was not created"));
            return false;
        }

        loadSavedSessionsToSidebar();
        applyAppTheme(QStringLiteral("system"));
        applyQuickToolbarVisibility(false);
        QMessageBox::information(
            this,
            QStringLiteral("Fresh config created"),
            QStringLiteral("A fresh empty dd-ssh.json was created.\n\nPrevious corrupt config moved to:\n%1")
                .arg(movedCorruptPath.isEmpty() ? QStringLiteral("(no previous config file was present)") : movedCorruptPath)
        );
        statusBar()->showMessage(QStringLiteral("Fresh dd-ssh.json created. Saved session list is empty."));
        return true;
    }

    statusBar()->showMessage(QStringLiteral("Config recovery: continuing read-only; dd-ssh.json was not overwritten"));
    return false;
}

void MainWindow::setupMenus()
{
    auto *fileMenu = menuBar()->addMenu("&File");

    auto *openConfigFolderAction = fileMenu->addAction("Open Config Folder");
    connect(openConfigFolderAction, &QAction::triggered, this, [this]() {
        openConfigFolder();
    });

    fileMenu->addSeparator();

    auto *exportConfigAction = fileMenu->addAction("Export Config...");
    connect(exportConfigAction, &QAction::triggered, this, [this]() {
        exportConfig();
    });

    auto *importConfigAction = fileMenu->addAction("Import Config...");
    connect(importConfigAction, &QAction::triggered, this, [this]() {
        importConfig();
    });

    auto *restoreBackupAction = fileMenu->addAction("Restore Latest Backup...");
    connect(restoreBackupAction, &QAction::triggered, this, [this]() {
        restoreLatestConfigBackup();
    });

    fileMenu->addSeparator();

    auto *exitAction = fileMenu->addAction("Exit");
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    auto *sessionMenu = menuBar()->addMenu("&Session");

    auto *newSessionAction = sessionMenu->addAction("New Session");
    connect(newSessionAction, &QAction::triggered, this, [this]() {
        showNewSessionDialog();
    });

    auto *connectAction = sessionMenu->addAction("Connect / Auth test");
    connect(connectAction, &QAction::triggered, this, [this]() {
        showManualConnectDialog();
    });

    auto *editSelectedAction = sessionMenu->addAction("Edit selected session");
    connect(editSelectedAction, &QAction::triggered, this, [this]() {
        editSelectedSession();
    });

    auto *toolsMenu = menuBar()->addMenu("&Tools");

    auto *multiExecAction = toolsMenu->addAction("Multi-Exec");
    connect(multiExecAction, &QAction::triggered, this, [this]() {
        statusBar()->showMessage("Multi-Exec placeholder clicked");
    });

    toolsMenu->addSeparator();

    auto *settingsMenuAction = toolsMenu->addAction("Settings");
    connect(settingsMenuAction, &QAction::triggered, this, [this]() {
        showSettingsDialog();
    });

    auto *helpMenu = menuBar()->addMenu("&Help");

    auto *aboutAction = helpMenu->addAction("About DD-SSH");
    connect(aboutAction, &QAction::triggered, this, [this]() {
        KnownHostsManager knownHosts;

        const QString aboutText =
            QStringLiteral("DD-SSH\n\n")
            + QStringLiteral("A clean cross-platform SSH client and session manager.\n\n")
            + QStringLiteral("Current phase: Windows build documentation and release build test.\n\n")
            + QStringLiteral("Version: ")
            + QCoreApplication::applicationVersion()
            + QStringLiteral("\n")
            + QStringLiteral("Codename: ")
            + QStringLiteral(DD_SSH_CODENAME_STRING)
            + QStringLiteral("\n")
            + QStringLiteral("Milestone: ")
            + QStringLiteral(DD_SSH_MILESTONE_STRING)
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
    auto *toolbar = addToolBar("Quick Action Toolbar");
    toolbar->setObjectName(QStringLiteral("quickActionToolbar"));
    toolbar->setMovable(false);
    m_mainToolBar = toolbar;

    auto *newSessionAction = toolbar->addAction("New Session");
    connect(newSessionAction, &QAction::triggered, this, [this]() {
        showNewSessionDialog();
    });

    auto *connectAction = toolbar->addAction("Connect");
    connect(connectAction, &QAction::triggered, this, [this]() {
        showManualConnectDialog();
    });

    toolbar->addSeparator();

    auto *multiExecAction = toolbar->addAction("Multi-Exec");
    connect(multiExecAction, &QAction::triggered, this, [this]() {
        statusBar()->showMessage("Multi-Exec placeholder clicked");
    });

    toolbar->addSeparator();

    auto *settingsAction = toolbar->addAction("Settings");
    connect(settingsAction, &QAction::triggered, this, [this]() {
        showSettingsDialog();
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

        if (widget == nullptr) {
            return;
        }

        if (auto *webTerminal = dynamic_cast<WebTerminalTab *>(widget)) {
            if (webTerminal->hasActiveShell()) {
                const QMessageBox::StandardButton decision = QMessageBox::question(
                    this,
                    QStringLiteral("Close active SSH terminal?"),
                    QStringLiteral("The SSH terminal '%1' is still connected.\n\nDisconnect and close this tab?")
                        .arg(webTerminal->displayName()),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::No
                );

                if (decision != QMessageBox::Yes) {
                    statusBar()->showMessage(QStringLiteral("Close tab cancelled: ") + webTerminal->displayName());
                    return;
                }

                webTerminal->requestDisconnect();
            }
        } else if (auto *basicTerminal = dynamic_cast<BasicTerminalTab *>(widget)) {
            if (basicTerminal->hasActiveShell()) {
                const QMessageBox::StandardButton decision = QMessageBox::question(
                    this,
                    QStringLiteral("Close active SSH shell?"),
                    QStringLiteral("The basic SSH shell '%1' is still connected.\n\nDisconnect and close this tab?")
                        .arg(basicTerminal->displayName()),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::No
                );

                if (decision != QMessageBox::Yes) {
                    statusBar()->showMessage(QStringLiteral("Close tab cancelled: ") + basicTerminal->displayName());
                    return;
                }

                basicTerminal->requestDisconnect();
            }
        }

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

        openSavedSessionWebTerminal(sessionId);
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
        auto *item = new QListWidgetItem("Config problem — sessions not loaded");
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        item->setToolTip(loadError);
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
        "DD-SSH — Andromeda\n\n"
        "A clean cross-platform SSH client and session manager.\n\n"
        "Double-click a saved session on the left to open the xterm.js terminal.\n\n"
        "Current milestone:\n"
        "MF 0.2 candidate — Real Terminal Foundation / Windows build validation\n\n"
        "Working now:\n"
        "- saved sessions loaded from dd-ssh.json\n"
        "- portable plaintext secrets in dd-ssh.json\n"
        "- password and private-key authentication\n"
        "- known_hosts trust handling\n"
        "- create/edit/delete saved sessions\n"
        "- duplicate saved-session warning\n"
        "- xterm.js terminal renderer with local bundled assets\n"
        "- SSH PTY resize sync\n"
        "- tested terminal apps: htop, nano, vim, top, clear\n"
        "- reconnect after disconnect/reboot\n"
        "- settings foundation with app theme, font settings, backups, and toolbar visibility\n"
        "- config import/export/restore and corrupt config recovery\n"
        "- native Windows build validated with Qt/MSVC/vcpkg/libssh\n\n"
        "Main menus:\n"
        "- File: Open Config Folder, Export Config, Import Config, Restore Latest Backup, Exit\n"
        "- Session: New Session, Connect / Auth test, Edit selected session\n"
        "- Tools: Multi-Exec placeholder, Settings\n"
        "- Help: About DD-SSH\n\n"
        "Documentation:\n"
        "- README.md is the public-alpha entry point\n"
        "- docs/GETTING_STARTED.md explains first build and first connection\n"
        "- docs/WINDOWS_BUILD.md explains native Windows build and release build testing\n"
        "- docs/USER_GUIDE.md explains menus and workflows\n"
        "- docs/USE_CASES.md explains practical usage scenarios\n"
        "- docs/CONFIG_MANAGEMENT.md explains backups/import/export/recovery\n"
        "- docs/SECURITY_NOTES.md explains plaintext secrets and known_hosts rules\n"
        "- docs/TEST_MATRIX.md tracks current Andromeda validation\n"
        "- docs/ROADMAP.md tracks future versions\n\n"
        "Next milestone:\n"
        "- app icon integration for Windows/Linux\n"
        "- WebEngine startup polish / first-terminal loading message\n"
        "- Windows deployment experiment with windeployqt\n"
        "- prepare v0.2.0-alpha milestone notes\n\n"
        "Codename roadmap:\n"
        "- 0.0.x — Launchpad / early prototype history\n"
        "- 0.1.x — Andromeda / current MF 0.2 candidate line\n"
        "- 0.2.x — Orion\n"
        "- 0.3.x — Vega\n"
        "- 0.4.x — Cassiopeia\n"
        "- 1.0.x — Apollo\n\n"
        "Detailed version history lives in docs/CHANGELOG.md.\n"
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
    QAction *openWebTerminalAction = menu.addAction("Open xterm.js terminal");
    QAction *connectAction = menu.addAction("Run auth test");
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

void MainWindow::editSelectedSession()
{
    if (m_sessionList == nullptr) {
        return;
    }

    QListWidgetItem *item = m_sessionList->currentItem();

    if (item == nullptr || item->data(Qt::UserRole).toString().trimmed().isEmpty()) {
        QMessageBox::information(
            this,
            QStringLiteral("No saved session selected"),
            QStringLiteral("Select a saved session in the sidebar first, then choose Session → Edit selected session.")
        );
        statusBar()->showMessage(QStringLiteral("Edit selected session: no saved session selected"));
        return;
    }

    editSavedSession(item->data(Qt::UserRole).toString());
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
        titleSuffix = QStringLiteral(" xterm");
    } else {
        terminal = new BasicTerminalTab(session, secretValue, this);
        titleSuffix = QStringLiteral(" shell");
    }

    const int tabIndex = m_tabs->addTab(terminal, session.name + titleSuffix);
    m_tabs->setCurrentIndex(tabIndex);

    if (auto *webTerminal = dynamic_cast<WebTerminalTab *>(terminal)) {
        connect(webTerminal, &WebTerminalTab::tabTitleChanged, this, [this, webTerminal](const QString &title) {
            const int index = m_tabs->indexOf(webTerminal);

            if (index >= 0) {
                m_tabs->setTabText(index, title);
            }
        });
        connect(webTerminal, &WebTerminalTab::lifecycleStatusChanged, this, [this](const QString &status) {
            statusBar()->showMessage(status);
        });
    } else if (auto *basicTerminal = dynamic_cast<BasicTerminalTab *>(terminal)) {
        connect(basicTerminal, &BasicTerminalTab::tabTitleChanged, this, [this, basicTerminal](const QString &title) {
            const int index = m_tabs->indexOf(basicTerminal);

            if (index >= 0) {
                m_tabs->setTabText(index, title);
            }
        });
        connect(basicTerminal, &BasicTerminalTab::lifecycleStatusChanged, this, [this](const QString &status) {
            statusBar()->showMessage(status);
        });
    }

    statusBar()->showMessage(
        useWebTerminal
            ? "Opening xterm.js terminal for " + tabTitle
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

void MainWindow::showSettingsDialog()
{
    ConfigManager config;
    QString loadError;
    AppSettings currentSettings = config.loadSettings(&loadError);

    if (!loadError.isEmpty()) {
        const ConfigInspection inspection = config.inspectConfig();

        if (inspection.hasProblem) {
            const bool recovered = showConfigRecoveryDialog(inspection);

            if (!recovered) {
                statusBar()->showMessage(QStringLiteral("Settings not opened because dd-ssh.json still needs recovery"));
                return;
            }

            loadError.clear();
            currentSettings = config.loadSettings(&loadError);

            if (!loadError.isEmpty()) {
                QMessageBox::warning(
                    this,
                    QStringLiteral("Could not load settings"),
                    loadError
                );
                statusBar()->showMessage(QStringLiteral("Could not load settings after config recovery"));
                return;
            }
        } else {
            QMessageBox::warning(
                this,
                QStringLiteral("Could not load settings"),
                loadError
            );
            statusBar()->showMessage(QStringLiteral("Could not load settings from dd-ssh.json"));
            return;
        }
    }

    SettingsDialog dialog(currentSettings, config.configFilePath(), this);

    if (dialog.exec() != QDialog::Accepted) {
        statusBar()->showMessage(QStringLiteral("Settings cancelled"));
        return;
    }

    QString saveError;

    const AppSettings newSettings = dialog.settings();

    if (!config.saveSettings(newSettings, &saveError)) {
        QMessageBox::warning(
            this,
            QStringLiteral("Could not save settings"),
            saveError
        );
        statusBar()->showMessage(QStringLiteral("Could not save settings to dd-ssh.json"));
        return;
    }

    applyAppTheme(newSettings.appTheme);
    applyQuickToolbarVisibility(newSettings.showQuickToolbar);
    statusBar()->showMessage(QStringLiteral("Settings saved to dd-ssh.json. App theme and quick toolbar visibility applied. New terminal font settings apply to newly opened terminals."));
}

void MainWindow::showNewSessionDialog()
{
    showConnectDialog(true);
}

void MainWindow::showManualConnectDialog()
{
    showConnectDialog(false);
}

void MainWindow::showConnectDialog(bool newSavedSessionMode)
{
    ConnectDialog dialog(this);
    dialog.setDialogMode(newSavedSessionMode
        ? ConnectDialog::DialogMode::NewSession
        : ConnectDialog::DialogMode::ManualConnect);

    if (dialog.exec() != QDialog::Accepted) {
        statusBar()->showMessage(newSavedSessionMode ? "New session dialog cancelled" : "Manual connect dialog cancelled");
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

    output += newSavedSessionMode
        ? QStringLiteral("DD-SSH new saved session auth test\n\n")
        : QStringLiteral("DD-SSH manual connection test\n\n");
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
