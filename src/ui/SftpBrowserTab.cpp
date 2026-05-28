#include "SftpBrowserTab.h"
#include "sftp/SftpProbe.h"
#include "core/AppLogger.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QModelIndex>
#include <QPushButton>
#include <QSplitter>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTreeView>
#include <QVariant>
#include <QVBoxLayout>

namespace {
QString formatSize(quint64 bytes)
{
    if (bytes < 1024ULL) {
        return QString::number(bytes) + QStringLiteral(" B");
    }

    const double value = static_cast<double>(bytes);

    if (bytes < 1024ULL * 1024ULL) {
        return QString::number(value / 1024.0, 'f', 1) + QStringLiteral(" KB");
    }

    if (bytes < 1024ULL * 1024ULL * 1024ULL) {
        return QString::number(value / (1024.0 * 1024.0), 'f', 1) + QStringLiteral(" MB");
    }

    return QString::number(value / (1024.0 * 1024.0 * 1024.0), 'f', 1) + QStringLiteral(" GB");
}

bool isDirectoryType(const QString &type)
{
    return type.compare(QStringLiteral("directory"), Qt::CaseInsensitive) == 0;
}

QTableWidgetItem *makeReadOnlyItem(const QString &text)
{
    auto *item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
}
} // namespace

SftpBrowserTab::SftpBrowserTab(
    const QString &sessionName,
    const QString &host,
    int port,
    const QString &username,
    SshAuthMethod authMethod,
    const QString &secretValue,
    const SshHostKeyExpectation &hostKeyExpectation,
    QWidget *parent
)
    : QWidget(parent)
    , m_sessionName(sessionName)
    , m_host(host)
    , m_port(port)
    , m_username(username)
    , m_authMethod(authMethod)
    , m_secretValue(secretValue)
    , m_hostKeyExpectation(hostKeyExpectation)
    , m_currentLocalPath(QDir::homePath())
{
    setupUi();
    refreshLocalDirectory();
    refreshRemoteDirectory();
}

QString SftpBrowserTab::displayName() const
{
    const QString baseName = m_sessionName.trimmed().isEmpty()
        ? QStringLiteral("SFTP browser")
        : m_sessionName.trimmed() + QStringLiteral(" files");

    return baseName
        + QStringLiteral(" (")
        + m_username
        + QStringLiteral("@")
        + m_host
        + QStringLiteral(":")
        + QString::number(m_port)
        + QStringLiteral(")");
}

void SftpBrowserTab::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    auto *headerLabel = new QLabel(
        QStringLiteral("Read-only two-panel file manager — local files ⇄ %1@%2:%3")
            .arg(m_username, m_host, QString::number(m_port)),
        this
    );
    headerLabel->setToolTip(QStringLiteral("This checkpoint can browse local and remote directories only. Upload, download, delete, rename, queue, and transfer progress actions are intentionally not implemented yet."));
    mainLayout->addWidget(headerLabel);

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setChildrenCollapsible(false);

    auto *localPanel = new QWidget(splitter);
    auto *localLayout = new QVBoxLayout(localPanel);
    localLayout->setContentsMargins(0, 0, 4, 0);
    localLayout->setSpacing(6);

    auto *localTitle = new QLabel(QStringLiteral("Local files (read-only)"), localPanel);
    localLayout->addWidget(localTitle);

    auto *localPathLayout = new QHBoxLayout();
    localPathLayout->addWidget(new QLabel(QStringLiteral("Local path:"), localPanel));

    m_localPathEdit = new QLineEdit(localPanel);
    m_localPathEdit->setText(m_currentLocalPath);
    m_localPathEdit->setPlaceholderText(QDir::homePath());
    localPathLayout->addWidget(m_localPathEdit, 1);

    m_localGoButton = new QPushButton(QStringLiteral("Go"), localPanel);
    localPathLayout->addWidget(m_localGoButton);

    m_localUpButton = new QPushButton(QStringLiteral("↑ Up"), localPanel);
    localPathLayout->addWidget(m_localUpButton);

    m_localRefreshButton = new QPushButton(QStringLiteral("Refresh"), localPanel);
    localPathLayout->addWidget(m_localRefreshButton);

    localLayout->addLayout(localPathLayout);

    m_localModel = new QFileSystemModel(this);
    m_localModel->setReadOnly(true);
    m_localModel->setFilter(QDir::AllEntries | QDir::AllDirs | QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot);

    m_localTree = new QTreeView(localPanel);
    m_localTree->setModel(m_localModel);
    m_localTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_localTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_localTree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_localTree->setAlternatingRowColors(false);
    m_localTree->setSortingEnabled(true);
    m_localTree->sortByColumn(0, Qt::AscendingOrder);
    m_localTree->header()->setStretchLastSection(false);
    m_localTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    localLayout->addWidget(m_localTree, 1);

    m_localStatusLabel = new QLabel(QStringLiteral("Local browser ready (read-only)"), localPanel);
    localLayout->addWidget(m_localStatusLabel);

    auto *remotePanel = new QWidget(splitter);
    auto *remoteLayout = new QVBoxLayout(remotePanel);
    remoteLayout->setContentsMargins(4, 0, 0, 0);
    remoteLayout->setSpacing(6);

    auto *remoteTitle = new QLabel(QStringLiteral("Remote files (read-only SFTP)"), remotePanel);
    remoteLayout->addWidget(remoteTitle);

    auto *remotePathLayout = new QHBoxLayout();
    remotePathLayout->addWidget(new QLabel(QStringLiteral("Remote path:"), remotePanel));

    m_remotePathEdit = new QLineEdit(remotePanel);
    m_remotePathEdit->setText(m_currentRemotePath);
    m_remotePathEdit->setPlaceholderText(QStringLiteral("., /home/user, /var/log ..."));
    remotePathLayout->addWidget(m_remotePathEdit, 1);

    m_remoteGoButton = new QPushButton(QStringLiteral("Go"), remotePanel);
    remotePathLayout->addWidget(m_remoteGoButton);

    m_remoteUpButton = new QPushButton(QStringLiteral("↑ Up"), remotePanel);
    remotePathLayout->addWidget(m_remoteUpButton);

    m_remoteRefreshButton = new QPushButton(QStringLiteral("Refresh"), remotePanel);
    remotePathLayout->addWidget(m_remoteRefreshButton);

    remoteLayout->addLayout(remotePathLayout);

    m_remoteTable = new QTableWidget(remotePanel);
    m_remoteTable->setColumnCount(5);
    m_remoteTable->setHorizontalHeaderLabels(QStringList{
        QStringLiteral("Name"),
        QStringLiteral("Type"),
        QStringLiteral("Size"),
        QStringLiteral("Modified"),
        QStringLiteral("Permissions")
    });
    m_remoteTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_remoteTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_remoteTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_remoteTable->setAlternatingRowColors(false);
    m_remoteTable->horizontalHeader()->setStretchLastSection(false);
    m_remoteTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_remoteTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_remoteTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_remoteTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_remoteTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_remoteTable->verticalHeader()->setVisible(false);
    m_remoteTable->setSortingEnabled(true);
    remoteLayout->addWidget(m_remoteTable, 1);

    m_remoteStatusLabel = new QLabel(QStringLiteral("Remote browser ready (read-only)"), remotePanel);
    remoteLayout->addWidget(m_remoteStatusLabel);

    splitter->addWidget(localPanel);
    splitter->addWidget(remotePanel);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);
    mainLayout->addWidget(splitter, 1);

    auto *transferNotice = new QLabel(
        QStringLiteral("Transfers are intentionally disabled in this checkpoint. Double-click folders to browse; selecting files does not upload or download yet."),
        this
    );
    transferNotice->setWordWrap(true);
    mainLayout->addWidget(transferNotice);

    connect(m_localRefreshButton, &QPushButton::clicked, this, [this]() {
        refreshLocalDirectory();
    });

    connect(m_localGoButton, &QPushButton::clicked, this, [this]() {
        openLocalPathFromEditor();
    });

    connect(m_localPathEdit, &QLineEdit::returnPressed, this, [this]() {
        openLocalPathFromEditor();
    });

    connect(m_localUpButton, &QPushButton::clicked, this, [this]() {
        goLocalUp();
    });

    connect(m_localTree, &QTreeView::doubleClicked, this, [this](const QModelIndex &index) {
        handleLocalDoubleClicked(index);
    });

    connect(m_remoteRefreshButton, &QPushButton::clicked, this, [this]() {
        refreshRemoteDirectory();
    });

    connect(m_remoteGoButton, &QPushButton::clicked, this, [this]() {
        openRemotePathFromEditor();
    });

    connect(m_remotePathEdit, &QLineEdit::returnPressed, this, [this]() {
        openRemotePathFromEditor();
    });

    connect(m_remoteUpButton, &QPushButton::clicked, this, [this]() {
        goRemoteUp();
    });

    connect(m_remoteTable, &QTableWidget::cellDoubleClicked, this, [this](int row, int column) {
        handleRemoteCellDoubleClicked(row, column);
    });
}

QString SftpBrowserTab::normalizedLocalPath(const QString &path) const
{
    const QString trimmed = path.trimmed();
    if (trimmed.isEmpty()) {
        return QDir::homePath();
    }

    return QDir::cleanPath(trimmed);
}

void SftpBrowserTab::setLocalPath(const QString &path)
{
    const QString normalized = normalizedLocalPath(path);
    const QFileInfo info(normalized);

    if (!info.exists()) {
        if (m_localStatusLabel != nullptr) {
            m_localStatusLabel->setText(QStringLiteral("Local path does not exist: ") + normalized);
        }
        QMessageBox::warning(this, QStringLiteral("Local path not found — DD-SSH"), QStringLiteral("Local path does not exist:\n%1").arg(normalized));
        return;
    }

    if (!info.isDir()) {
        if (m_localStatusLabel != nullptr) {
            m_localStatusLabel->setText(QStringLiteral("Selected local file: ") + normalized + QStringLiteral(" — transfers are not implemented yet"));
        }
        return;
    }

    m_currentLocalPath = normalized;

    if (m_localPathEdit != nullptr) {
        m_localPathEdit->setText(m_currentLocalPath);
    }

    refreshLocalDirectory();
}

void SftpBrowserTab::openLocalPathFromEditor()
{
    if (m_localPathEdit == nullptr) {
        return;
    }

    setLocalPath(m_localPathEdit->text());
}

void SftpBrowserTab::goLocalUp()
{
    QDir dir(m_currentLocalPath);

    if (!dir.cdUp()) {
        if (m_localStatusLabel != nullptr) {
            m_localStatusLabel->setText(QStringLiteral("Already at local filesystem root"));
        }
        return;
    }

    setLocalPath(dir.absolutePath());
}

void SftpBrowserTab::refreshLocalDirectory()
{
    if (m_localModel == nullptr || m_localTree == nullptr) {
        return;
    }

    const QString path = normalizedLocalPath(m_currentLocalPath);
    const QFileInfo info(path);

    if (!info.exists() || !info.isDir()) {
        if (m_localStatusLabel != nullptr) {
            m_localStatusLabel->setText(QStringLiteral("Cannot open local path: ") + path);
        }
        return;
    }

    m_currentLocalPath = path;

    const QModelIndex rootIndex = m_localModel->setRootPath(m_currentLocalPath);
    m_localTree->setRootIndex(rootIndex);

    if (m_localPathEdit != nullptr) {
        m_localPathEdit->setText(m_currentLocalPath);
    }

    if (m_localStatusLabel != nullptr) {
        m_localStatusLabel->setText(QStringLiteral("Local: ") + m_currentLocalPath + QStringLiteral(" (read-only)"));
    }
}

void SftpBrowserTab::handleLocalDoubleClicked(const QModelIndex &index)
{
    if (m_localModel == nullptr || !index.isValid()) {
        return;
    }

    const QString selectedPath = m_localModel->filePath(index);
    const QFileInfo info(selectedPath);

    if (info.isDir()) {
        setLocalPath(selectedPath);
        return;
    }

    if (m_localStatusLabel != nullptr) {
        m_localStatusLabel->setText(QStringLiteral("Selected local file: ") + selectedPath + QStringLiteral(" — transfers are not implemented yet"));
    }
}

void SftpBrowserTab::setRemoteBusy(bool busy)
{
    if (m_remoteGoButton != nullptr) {
        m_remoteGoButton->setEnabled(!busy);
    }

    if (m_remoteUpButton != nullptr) {
        m_remoteUpButton->setEnabled(!busy);
    }

    if (m_remoteRefreshButton != nullptr) {
        m_remoteRefreshButton->setEnabled(!busy);
    }

    if (m_remotePathEdit != nullptr) {
        m_remotePathEdit->setEnabled(!busy);
    }

    if (m_remoteTable != nullptr) {
        m_remoteTable->setEnabled(!busy);
    }
}

QString SftpBrowserTab::normalizedRemotePath(const QString &path) const
{
    const QString trimmed = path.trimmed();
    return trimmed.isEmpty() ? QStringLiteral(".") : trimmed;
}

QString SftpBrowserTab::joinedRemotePath(const QString &basePath, const QString &entryName) const
{
    const QString name = entryName.trimmed();

    if (name.isEmpty() || name == QStringLiteral(".")) {
        return normalizedRemotePath(basePath);
    }

    if (name == QStringLiteral("..")) {
        return parentRemotePath(basePath);
    }

    QString base = normalizedRemotePath(basePath);

    if (base == QStringLiteral(".")) {
        return name;
    }

    while (base.size() > 1 && base.endsWith(QLatin1Char('/'))) {
        base.chop(1);
    }

    if (base == QStringLiteral("/")) {
        return QStringLiteral("/") + name;
    }

    return base + QStringLiteral("/") + name;
}

QString SftpBrowserTab::parentRemotePath(const QString &path) const
{
    QString normalized = normalizedRemotePath(path);

    while (normalized.size() > 1 && normalized.endsWith(QLatin1Char('/'))) {
        normalized.chop(1);
    }

    if (normalized == QStringLiteral(".") || normalized == QStringLiteral("/")) {
        return normalized;
    }

    const int slashIndex = normalized.lastIndexOf(QLatin1Char('/'));

    if (slashIndex < 0) {
        return QStringLiteral(".");
    }

    if (slashIndex == 0) {
        return QStringLiteral("/");
    }

    return normalized.left(slashIndex);
}

QString SftpBrowserTab::statusPrefix() const
{
    const QString safeName = m_sessionName.trimmed().isEmpty() ? QStringLiteral("session") : m_sessionName.trimmed();
    return QStringLiteral("SFTP %1: ").arg(safeName);
}

void SftpBrowserTab::refreshRemoteDirectory()
{
    const QString requestedPath = normalizedRemotePath(m_currentRemotePath);

    AppLogger::info(QStringLiteral("SFTP browser remote directory refresh requested: session=\"") + m_sessionName
        + QStringLiteral("\", host=") + m_host
        + QStringLiteral(", port=") + QString::number(m_port)
        + QStringLiteral(", path=") + requestedPath);

    setRemoteBusy(true);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (m_remoteStatusLabel != nullptr) {
        m_remoteStatusLabel->setText(statusPrefix() + QStringLiteral("loading ") + requestedPath + QStringLiteral(" ..."));
    }

    QApplication::processEvents();

    const SftpProbeResult result = SftpProbe::listRemoteDirectory(
        m_host,
        m_port,
        m_username,
        m_authMethod,
        m_secretValue,
        m_hostKeyExpectation,
        requestedPath
    );

    QApplication::restoreOverrideCursor();
    setRemoteBusy(false);

    if (!result.success) {
        AppLogger::warn(QStringLiteral("SFTP browser remote directory refresh failed: session=\"") + m_sessionName
            + QStringLiteral("\", path=") + requestedPath
            + QStringLiteral(", message=") + result.message
            + QStringLiteral(", error=") + result.error);

        if (m_remoteStatusLabel != nullptr) {
            m_remoteStatusLabel->setText(statusPrefix() + QStringLiteral("failed to load ") + requestedPath + QStringLiteral(" — ") + result.message);
        }

        QMessageBox::warning(
            this,
            QStringLiteral("SFTP browser failed — DD-SSH"),
            QStringLiteral("Could not list remote path:\n%1\n\n%2\n\nError:\n%3")
                .arg(requestedPath, result.message, result.error)
        );
        return;
    }

    m_currentRemotePath = result.remotePath;

    if (m_remotePathEdit != nullptr) {
        m_remotePathEdit->setText(m_currentRemotePath);
    }

    populateRemoteTable(result.entries);

    if (m_remoteStatusLabel != nullptr) {
        m_remoteStatusLabel->setText(statusPrefix() + QStringLiteral("loaded ") + m_currentRemotePath
            + QStringLiteral(" — ") + QString::number(result.entries.size()) + QStringLiteral(" entries (read-only)"));
    }

    AppLogger::info(QStringLiteral("SFTP browser remote directory refresh successful: session=\"") + m_sessionName
        + QStringLiteral("\", path=") + m_currentRemotePath
        + QStringLiteral(", entries=") + QString::number(result.entries.size()));
}

void SftpBrowserTab::openRemotePathFromEditor()
{
    if (m_remotePathEdit == nullptr) {
        return;
    }

    m_currentRemotePath = normalizedRemotePath(m_remotePathEdit->text());
    refreshRemoteDirectory();
}

void SftpBrowserTab::goRemoteUp()
{
    m_currentRemotePath = parentRemotePath(m_currentRemotePath);

    if (m_remotePathEdit != nullptr) {
        m_remotePathEdit->setText(m_currentRemotePath);
    }

    refreshRemoteDirectory();
}

void SftpBrowserTab::handleRemoteCellDoubleClicked(int row, int)
{
    if (m_remoteTable == nullptr || row < 0 || row >= m_remoteTable->rowCount()) {
        return;
    }

    QTableWidgetItem *nameItem = m_remoteTable->item(row, 0);
    QTableWidgetItem *typeItem = m_remoteTable->item(row, 1);

    if (nameItem == nullptr || typeItem == nullptr) {
        return;
    }

    const QString name = nameItem->data(Qt::UserRole).toString();
    const QString type = typeItem->data(Qt::UserRole).toString();

    if (!isDirectoryType(type)) {
        if (m_remoteStatusLabel != nullptr) {
            m_remoteStatusLabel->setText(statusPrefix() + QStringLiteral("selected remote file ") + name + QStringLiteral(" — transfers are not implemented yet"));
        }
        return;
    }

    const QString nextPath = joinedRemotePath(m_currentRemotePath, name);

    if (nextPath == m_currentRemotePath && name == QStringLiteral(".")) {
        return;
    }

    m_currentRemotePath = nextPath;

    if (m_remotePathEdit != nullptr) {
        m_remotePathEdit->setText(m_currentRemotePath);
    }

    refreshRemoteDirectory();
}

void SftpBrowserTab::populateRemoteTable(const QList<SftpRemoteEntry> &entries)
{
    if (m_remoteTable == nullptr) {
        return;
    }

    m_remoteTable->setSortingEnabled(false);
    m_remoteTable->clearContents();
    m_remoteTable->setRowCount(entries.size());

    for (int row = 0; row < entries.size(); ++row) {
        const SftpRemoteEntry &entry = entries.at(row);

        QTableWidgetItem *nameItem = makeReadOnlyItem(entry.name);
        nameItem->setData(Qt::UserRole, entry.name);

        QTableWidgetItem *typeItem = makeReadOnlyItem(entry.type);
        typeItem->setData(Qt::UserRole, entry.type);

        QTableWidgetItem *sizeItem = makeReadOnlyItem(formatSize(entry.sizeBytes));
        sizeItem->setData(Qt::UserRole, QVariant::fromValue<qulonglong>(entry.sizeBytes));
        sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        QTableWidgetItem *modifiedItem = makeReadOnlyItem(entry.modifiedTime);
        QTableWidgetItem *permissionsItem = makeReadOnlyItem(entry.permissions);

        m_remoteTable->setItem(row, 0, nameItem);
        m_remoteTable->setItem(row, 1, typeItem);
        m_remoteTable->setItem(row, 2, sizeItem);
        m_remoteTable->setItem(row, 3, modifiedItem);
        m_remoteTable->setItem(row, 4, permissionsItem);
    }

    m_remoteTable->setSortingEnabled(true);
    m_remoteTable->sortItems(0, Qt::AscendingOrder);
}
