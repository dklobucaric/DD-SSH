#include "SftpBrowserTab.h"
#include "sftp/SftpProbe.h"
#include "core/AppLogger.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>
#include <QModelIndex>
#include <QProgressDialog>
#include <QPushButton>
#include <QSplitter>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTreeView>
#include <QVariant>
#include <QVBoxLayout>

namespace {
QString formatUnitValue(double value, const QString &suffix)
{
    QString number = QString::number(value, 'f', 1);
    if (number.endsWith(QStringLiteral(".0"))) {
        number.chop(2);
    }
    return number + QStringLiteral(" ") + suffix;
}

QString formatSize(quint64 bytes)
{
    if (bytes < 1024ULL) {
        return QString::number(bytes) + QStringLiteral(" B");
    }

    const double value = static_cast<double>(bytes);

    if (bytes < 1024ULL * 1024ULL) {
        return formatUnitValue(value / 1024.0, QStringLiteral("KB"));
    }

    if (bytes < 1024ULL * 1024ULL * 1024ULL) {
        return formatUnitValue(value / (1024.0 * 1024.0), QStringLiteral("MB"));
    }

    return formatUnitValue(value / (1024.0 * 1024.0 * 1024.0), QStringLiteral("GB"));
}

QString formatRawBytes(quint64 bytes)
{
    return QLocale(QLocale::English).toString(static_cast<qulonglong>(bytes)) + QStringLiteral(" bytes");
}

QString formatDuration(qint64 elapsedMs)
{
    if (elapsedMs < 0) {
        elapsedMs = 0;
    }

    const qint64 totalSeconds = elapsedMs / 1000;
    const qint64 hours = totalSeconds / 3600;
    const qint64 minutes = (totalSeconds % 3600) / 60;
    const qint64 seconds = totalSeconds % 60;

    if (hours > 0) {
        return QStringLiteral("%1:%2:%3")
            .arg(hours, 2, 10, QLatin1Char('0'))
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0'));
    }

    return QStringLiteral("%1:%2")
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'));
}

QString formatTransferRate(quint64 bytesTransferred, qint64 elapsedMs)
{
    if (bytesTransferred == 0 || elapsedMs <= 0) {
        return QStringLiteral("calculating ...");
    }

    const double bytesPerSecond = static_cast<double>(bytesTransferred) * 1000.0 / static_cast<double>(elapsedMs);
    return formatSize(static_cast<quint64>(bytesPerSecond)) + QStringLiteral("/s");
}

QString formatTransferProgressLabel(
    const QString &action,
    const QString &message,
    quint64 bytesTransferred,
    quint64 totalBytes,
    qint64 elapsedMs
)
{
    const QString totalText = totalBytes > 0 ? formatSize(totalBytes) : QStringLiteral("unknown");
    const int percent = totalBytes > 0
        ? static_cast<int>(qMin<quint64>(100, (bytesTransferred * 100ULL) / totalBytes))
        : 0;

    QStringList lines;
    lines << QStringLiteral("%1 — %2").arg(action, message);
    lines << QStringLiteral("Transferred: %1 / %2").arg(formatSize(bytesTransferred), totalText);

    if (totalBytes > 0) {
        lines << QStringLiteral("Progress: %1%").arg(percent);
    } else {
        lines << QStringLiteral("Progress: unknown total size");
    }

    lines << QStringLiteral("Speed: %1").arg(formatTransferRate(bytesTransferred, elapsedMs));
    lines << QStringLiteral("Elapsed: %1").arg(formatDuration(elapsedMs));

    return lines.join(QLatin1Char('\n'));
}

QString formatTransferSummary(quint64 bytesTransferred, quint64 totalBytes, qint64 elapsedMs)
{
    QString summary = QStringLiteral("Transferred: %1 (%2)\nElapsed: %3\nAverage speed: %4")
        .arg(formatSize(bytesTransferred), formatRawBytes(bytesTransferred), formatDuration(elapsedMs), formatTransferRate(bytesTransferred, elapsedMs));

    if (totalBytes > 0) {
        summary += QStringLiteral("\nExpected size: %1 (%2)")
            .arg(formatSize(totalBytes), formatRawBytes(totalBytes));
    }

    return summary;
}

class SizeTableWidgetItem final : public QTableWidgetItem
{
public:
    explicit SizeTableWidgetItem(quint64 bytes)
        : QTableWidgetItem(formatSize(bytes))
    {
        setData(Qt::UserRole, QVariant::fromValue<qulonglong>(bytes));
        setFlags(flags() & ~Qt::ItemIsEditable);
        setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }

    bool operator<(const QTableWidgetItem &other) const override
    {
        return data(Qt::UserRole).toULongLong() < other.data(Qt::UserRole).toULongLong();
    }
};

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
        QStringLiteral("Two-panel file manager — local files ⇄ %1@%2:%3")
            .arg(m_username, m_host, QString::number(m_port)),
        this
    );
    headerLabel->setToolTip(QStringLiteral("This checkpoint supports single-file download and upload with progress, speed, elapsed-time, completion, and cancel feedback. Delete, rename, queue, sync, and folder transfer actions are intentionally not implemented yet."));
    mainLayout->addWidget(headerLabel);

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setChildrenCollapsible(false);

    auto *localPanel = new QWidget(splitter);
    auto *localLayout = new QVBoxLayout(localPanel);
    localLayout->setContentsMargins(0, 0, 4, 0);
    localLayout->setSpacing(6);

    auto *localTitle = new QLabel(QStringLiteral("Local files (download target / upload source)"), localPanel);
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

    m_localUploadButton = new QPushButton(QStringLiteral("Upload selected"), localPanel);
    m_localUploadButton->setToolTip(QStringLiteral("Upload the selected local file into the currently open remote folder. Folder upload, queue, and sync are not implemented yet."));
    localPathLayout->addWidget(m_localUploadButton);

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

    auto *remoteTitle = new QLabel(QStringLiteral("Remote files (SFTP — download/upload)"), remotePanel);
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

    m_remoteDownloadButton = new QPushButton(QStringLiteral("Download selected"), remotePanel);
    m_remoteDownloadButton->setToolTip(QStringLiteral("Download the selected remote file into the currently open local folder. Folder download is not implemented yet."));
    remotePathLayout->addWidget(m_remoteDownloadButton);

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
        QStringLiteral("Single-file download and upload are enabled with progress, speed, elapsed-time, completion, and cancel feedback. Delete, rename, folder transfer, queue, and sync actions are intentionally disabled."),
        this
    );
    transferNotice->setWordWrap(true);
    mainLayout->addWidget(transferNotice);

    connect(m_localRefreshButton, &QPushButton::clicked, this, [this]() {
        refreshLocalDirectory();
    });

    connect(m_localUploadButton, &QPushButton::clicked, this, [this]() {
        uploadSelectedLocalFile();
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

    connect(m_remoteDownloadButton, &QPushButton::clicked, this, [this]() {
        downloadSelectedRemoteFile();
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
            m_localStatusLabel->setText(QStringLiteral("Selected local file: ") + normalized + QStringLiteral(" — use Upload selected to send it into the current remote folder"));
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
        m_localStatusLabel->setText(QStringLiteral("Selected local file: ") + selectedPath + QStringLiteral(" — use Upload selected to send it into the current remote folder"));
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

    if (m_remoteDownloadButton != nullptr) {
        m_remoteDownloadButton->setEnabled(!busy);
    }

    if (m_localUploadButton != nullptr) {
        m_localUploadButton->setEnabled(!busy);
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
            m_remoteStatusLabel->setText(statusPrefix() + QStringLiteral("selected remote file ") + name + QStringLiteral(" — use Download selected to save it into the current local folder"));
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

void SftpBrowserTab::downloadSelectedRemoteFile()
{
    if (m_remoteTable == nullptr) {
        return;
    }

    const QList<QTableWidgetSelectionRange> selections = m_remoteTable->selectedRanges();

    if (selections.isEmpty()) {
        QMessageBox::information(
            this,
            QStringLiteral("No remote file selected — DD-SSH"),
            QStringLiteral("Select one remote file first, then click Download selected.")
        );
        return;
    }

    const int row = selections.first().topRow();

    QTableWidgetItem *nameItem = m_remoteTable->item(row, 0);
    QTableWidgetItem *typeItem = m_remoteTable->item(row, 1);

    if (nameItem == nullptr || typeItem == nullptr) {
        return;
    }

    const QString name = nameItem->data(Qt::UserRole).toString();
    const QString type = typeItem->data(Qt::UserRole).toString();

    if (name.trimmed().isEmpty() || name == QStringLiteral(".") || name == QStringLiteral("..")) {
        QMessageBox::information(
            this,
            QStringLiteral("Cannot download this entry — DD-SSH"),
            QStringLiteral("The selected remote entry is not a regular file.")
        );
        return;
    }

    if (isDirectoryType(type)) {
        QMessageBox::information(
            this,
            QStringLiteral("Folder download not implemented — DD-SSH"),
            QStringLiteral("This checkpoint downloads one selected remote file only. Folder transfer is planned for a later checkpoint.")
        );
        return;
    }

    if (type.compare(QStringLiteral("file"), Qt::CaseInsensitive) != 0
        && type.compare(QStringLiteral("symlink"), Qt::CaseInsensitive) != 0) {
        const QMessageBox::StandardButton decision = QMessageBox::question(
            this,
            QStringLiteral("Download unusual remote entry? — DD-SSH"),
            QStringLiteral("The selected remote entry type is '%1'.\n\nDownload it anyway?").arg(type),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );

        if (decision != QMessageBox::Yes) {
            return;
        }
    }

    const QFileInfo localFolderInfo(m_currentLocalPath);

    if (!localFolderInfo.exists() || !localFolderInfo.isDir()) {
        QMessageBox::warning(
            this,
            QStringLiteral("Local destination unavailable — DD-SSH"),
            QStringLiteral("The current local destination is not a valid folder:\n%1").arg(m_currentLocalPath)
        );
        return;
    }

    const QString remoteFilePath = joinedRemotePath(m_currentRemotePath, name);
    const QString localTargetPath = QDir(m_currentLocalPath).filePath(QFileInfo(name).fileName());

    if (QFileInfo::exists(localTargetPath)) {
        const QMessageBox::StandardButton overwriteDecision = QMessageBox::warning(
            this,
            QStringLiteral("Overwrite local file? — DD-SSH"),
            QStringLiteral("The local file already exists:\n%1\n\nOverwrite it with the selected remote file?" ).arg(localTargetPath),
            QMessageBox::Yes | QMessageBox::Cancel,
            QMessageBox::Cancel
        );

        if (overwriteDecision != QMessageBox::Yes) {
            if (m_localStatusLabel != nullptr) {
                m_localStatusLabel->setText(QStringLiteral("Download cancelled before overwrite: ") + localTargetPath);
            }
            return;
        }
    }

    AppLogger::info(QStringLiteral("SFTP browser download requested: session=\"") + m_sessionName
        + QStringLiteral("\", remotePath=") + remoteFilePath
        + QStringLiteral(", localPath=") + localTargetPath);

    QProgressDialog progress(
        QStringLiteral("Downloading %1 ...").arg(name),
        QStringLiteral("Cancel"),
        0,
        100,
        this
    );
    progress.setWindowTitle(QStringLiteral("SFTP download — DD-SSH"));
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setMinimumDuration(0);
    progress.setValue(0);

    setRemoteBusy(true);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (m_remoteStatusLabel != nullptr) {
        m_remoteStatusLabel->setText(statusPrefix() + QStringLiteral("downloading ") + remoteFilePath + QStringLiteral(" ..."));
    }

    bool progressWasCancelled = false;
    QElapsedTimer transferTimer;
    transferTimer.start();

    const SftpDownloadResult result = SftpProbe::downloadRemoteFile(
        m_host,
        m_port,
        m_username,
        m_authMethod,
        m_secretValue,
        m_hostKeyExpectation,
        remoteFilePath,
        localTargetPath,
        [&progress, &progressWasCancelled, &transferTimer](quint64 bytesTransferred, quint64 totalBytes, const QString &message) -> bool {
            int percent = 0;

            if (totalBytes > 0) {
                percent = static_cast<int>(qMin<quint64>(100, (bytesTransferred * 100ULL) / totalBytes));
            } else if (bytesTransferred > 0) {
                percent = 50;
            }

            progress.setValue(percent);
            progress.setLabelText(
                formatTransferProgressLabel(
                    QStringLiteral("Download"),
                    message,
                    bytesTransferred,
                    totalBytes,
                    transferTimer.elapsed()
                )
            );
            QApplication::processEvents();

            if (progress.wasCanceled()) {
                progressWasCancelled = true;
                return false;
            }

            return true;
        }
    );

    QApplication::restoreOverrideCursor();
    setRemoteBusy(false);

    const qint64 elapsedMs = transferTimer.elapsed();

    if (result.success) {
        progress.setValue(100);
        refreshLocalDirectory();

        if (m_localStatusLabel != nullptr) {
            m_localStatusLabel->setText(QStringLiteral("Downloaded remote file to: ") + localTargetPath);
        }

        if (m_remoteStatusLabel != nullptr) {
            m_remoteStatusLabel->setText(statusPrefix() + QStringLiteral("downloaded ") + name + QStringLiteral(" — ") + formatSize(result.bytesTransferred));
        }

        QMessageBox::information(
            this,
            QStringLiteral("Download complete — DD-SSH"),
            QStringLiteral("Downloaded remote file:\n%1\n\nTo local file:\n%2\n\nDownloaded: %3 (%4)\nElapsed: %5\nAverage speed: %6")
                .arg(
                    remoteFilePath,
                    localTargetPath,
                    formatSize(result.bytesTransferred),
                    formatRawBytes(result.bytesTransferred),
                    formatDuration(elapsedMs),
                    formatTransferRate(result.bytesTransferred, elapsedMs)
                )
        );
        return;
    }

    if (result.cancelled || progressWasCancelled) {
        if (m_remoteStatusLabel != nullptr) {
            m_remoteStatusLabel->setText(statusPrefix() + QStringLiteral("download cancelled: ") + remoteFilePath);
        }

        QMessageBox::information(
            this,
            QStringLiteral("Download cancelled — DD-SSH"),
            QStringLiteral("Download cancelled by user.\n\nRemote file:\n%1\n\nLocal target was not replaced because DD-SSH uses a safe temporary download file.\n\n%2")
                .arg(remoteFilePath, formatTransferSummary(result.bytesTransferred, result.totalBytes, elapsedMs))
        );
        return;
    }

    if (m_remoteStatusLabel != nullptr) {
        m_remoteStatusLabel->setText(statusPrefix() + QStringLiteral("download failed: ") + result.message);
    }

    QMessageBox::warning(
        this,
        QStringLiteral("Download failed — DD-SSH"),
        QStringLiteral("Could not download remote file:\n%1\n\nTarget:\n%2\n\n%3\n\nError:\n%4")
            .arg(remoteFilePath, localTargetPath, result.message, result.error)
    );
}


void SftpBrowserTab::uploadSelectedLocalFile()
{
    if (m_localTree == nullptr || m_localModel == nullptr) {
        return;
    }

    const QModelIndexList selectedRows = m_localTree->selectionModel() != nullptr
        ? m_localTree->selectionModel()->selectedRows(0)
        : QModelIndexList{};

    QModelIndex selectedIndex;

    if (!selectedRows.isEmpty()) {
        selectedIndex = selectedRows.first();
    } else {
        selectedIndex = m_localTree->currentIndex();
        if (selectedIndex.isValid() && selectedIndex.column() != 0) {
            selectedIndex = selectedIndex.siblingAtColumn(0);
        }
    }

    if (!selectedIndex.isValid()) {
        QMessageBox::information(
            this,
            QStringLiteral("No local file selected — DD-SSH"),
            QStringLiteral("Select one local file first, then click Upload selected.")
        );
        return;
    }

    const QString localFilePath = m_localModel->filePath(selectedIndex);
    const QFileInfo localInfo(localFilePath);

    if (!localInfo.exists()) {
        QMessageBox::warning(
            this,
            QStringLiteral("Local file not found — DD-SSH"),
            QStringLiteral("The selected local file no longer exists:\n%1").arg(localFilePath)
        );
        return;
    }

    if (localInfo.isDir()) {
        QMessageBox::information(
            this,
            QStringLiteral("Folder upload not implemented — DD-SSH"),
            QStringLiteral("This checkpoint uploads one selected local file only. Folder transfer is planned for a later checkpoint.")
        );
        return;
    }

    if (!localInfo.isFile()) {
        const QMessageBox::StandardButton decision = QMessageBox::question(
            this,
            QStringLiteral("Upload unusual local entry? — DD-SSH"),
            QStringLiteral("The selected local entry is not reported as a regular file:\n%1\n\nUpload it anyway?").arg(localFilePath),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );

        if (decision != QMessageBox::Yes) {
            return;
        }
    }

    const QString fileName = localInfo.fileName();

    if (fileName.trimmed().isEmpty()) {
        QMessageBox::warning(
            this,
            QStringLiteral("Invalid local filename — DD-SSH"),
            QStringLiteral("Could not derive a remote filename from the selected local path.")
        );
        return;
    }

    const QString remoteTargetPath = joinedRemotePath(m_currentRemotePath, fileName);
    bool remoteExistsInCurrentListing = false;
    bool remoteTargetIsDirectory = false;

    if (m_remoteTable != nullptr) {
        for (int row = 0; row < m_remoteTable->rowCount(); ++row) {
            QTableWidgetItem *nameItem = m_remoteTable->item(row, 0);
            QTableWidgetItem *typeItem = m_remoteTable->item(row, 1);

            if (nameItem == nullptr || typeItem == nullptr) {
                continue;
            }

            const QString remoteName = nameItem->data(Qt::UserRole).toString();

            if (remoteName == fileName) {
                remoteExistsInCurrentListing = true;
                remoteTargetIsDirectory = isDirectoryType(typeItem->data(Qt::UserRole).toString());
                break;
            }
        }
    }

    bool allowOverwrite = false;

    if (remoteExistsInCurrentListing) {
        if (remoteTargetIsDirectory) {
            QMessageBox::warning(
                this,
                QStringLiteral("Remote target is a folder — DD-SSH"),
                QStringLiteral("A remote folder with the same name already exists:\n%1\n\nChoose a different remote path or rename the local file before uploading.").arg(remoteTargetPath)
            );
            return;
        }

        const QMessageBox::StandardButton overwriteDecision = QMessageBox::warning(
            this,
            QStringLiteral("Overwrite remote file? — DD-SSH"),
            QStringLiteral("The remote file already exists:\n%1\n\nOverwrite it with the selected local file?" ).arg(remoteTargetPath),
            QMessageBox::Yes | QMessageBox::Cancel,
            QMessageBox::Cancel
        );

        if (overwriteDecision != QMessageBox::Yes) {
            if (m_remoteStatusLabel != nullptr) {
                m_remoteStatusLabel->setText(statusPrefix() + QStringLiteral("upload cancelled before overwrite: ") + remoteTargetPath);
            }
            return;
        }

        allowOverwrite = true;
    }

    AppLogger::info(QStringLiteral("SFTP browser upload requested: session=\"") + m_sessionName
        + QStringLiteral("\", localPath=") + localFilePath
        + QStringLiteral(", remotePath=") + remoteTargetPath);

    QProgressDialog progress(
        QStringLiteral("Uploading %1 ...").arg(fileName),
        QStringLiteral("Cancel"),
        0,
        100,
        this
    );
    progress.setWindowTitle(QStringLiteral("SFTP upload — DD-SSH"));
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setMinimumDuration(0);
    progress.setValue(0);

    setRemoteBusy(true);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (m_remoteStatusLabel != nullptr) {
        m_remoteStatusLabel->setText(statusPrefix() + QStringLiteral("uploading ") + remoteTargetPath + QStringLiteral(" ..."));
    }

    bool progressWasCancelled = false;
    QElapsedTimer transferTimer;
    transferTimer.start();

    const SftpUploadResult result = SftpProbe::uploadLocalFile(
        m_host,
        m_port,
        m_username,
        m_authMethod,
        m_secretValue,
        m_hostKeyExpectation,
        localFilePath,
        remoteTargetPath,
        allowOverwrite,
        [&progress, &progressWasCancelled, &transferTimer](quint64 bytesTransferred, quint64 totalBytes, const QString &message) -> bool {
            int percent = 0;

            if (totalBytes > 0) {
                percent = static_cast<int>(qMin<quint64>(100, (bytesTransferred * 100ULL) / totalBytes));
            } else if (bytesTransferred > 0) {
                percent = 50;
            }

            progress.setValue(percent);
            progress.setLabelText(
                formatTransferProgressLabel(
                    QStringLiteral("Upload"),
                    message,
                    bytesTransferred,
                    totalBytes,
                    transferTimer.elapsed()
                )
            );
            QApplication::processEvents();

            if (progress.wasCanceled()) {
                progressWasCancelled = true;
                return false;
            }

            return true;
        }
    );

    QApplication::restoreOverrideCursor();
    setRemoteBusy(false);

    const qint64 elapsedMs = transferTimer.elapsed();

    if (result.success) {
        progress.setValue(100);
        refreshRemoteDirectory();

        if (m_localStatusLabel != nullptr) {
            m_localStatusLabel->setText(QStringLiteral("Uploaded local file: ") + localFilePath);
        }

        if (m_remoteStatusLabel != nullptr) {
            m_remoteStatusLabel->setText(statusPrefix() + QStringLiteral("uploaded ") + fileName + QStringLiteral(" — ") + formatSize(result.bytesTransferred));
        }

        QMessageBox::information(
            this,
            QStringLiteral("Upload complete — DD-SSH"),
            QStringLiteral("Uploaded local file:\n%1\n\nTo remote file:\n%2\n\nUploaded: %3 (%4)\nElapsed: %5\nAverage speed: %6")
                .arg(
                    localFilePath,
                    remoteTargetPath,
                    formatSize(result.bytesTransferred),
                    formatRawBytes(result.bytesTransferred),
                    formatDuration(elapsedMs),
                    formatTransferRate(result.bytesTransferred, elapsedMs)
                )
        );
        return;
    }

    if (result.remoteAlreadyExists && !allowOverwrite && !remoteExistsInCurrentListing) {
        QMessageBox::warning(
            this,
            QStringLiteral("Remote file exists — DD-SSH"),
            QStringLiteral("The remote file already exists, but it was not visible in the current listing or the listing was stale:\n%1\n\nRefresh the remote folder and try again if you want to overwrite it.").arg(remoteTargetPath)
        );
        if (m_remoteStatusLabel != nullptr) {
            m_remoteStatusLabel->setText(statusPrefix() + QStringLiteral("upload stopped: remote file already exists"));
        }
        refreshRemoteDirectory();
        return;
    }

    if (result.cancelled || progressWasCancelled) {
        if (m_remoteStatusLabel != nullptr) {
            m_remoteStatusLabel->setText(statusPrefix() + QStringLiteral("upload cancelled: ") + remoteTargetPath + QStringLiteral(" — partial remote file may remain"));
        }
        refreshRemoteDirectory();
        QMessageBox::information(
            this,
            QStringLiteral("Upload cancelled — DD-SSH"),
            QStringLiteral("Upload cancelled by user.\n\nLocal file:\n%1\n\nRemote target:\n%2\n\nA partial remote file may remain on the server. Refresh/check the remote folder before retrying.\n\n%3")
                .arg(localFilePath, remoteTargetPath, formatTransferSummary(result.bytesTransferred, result.totalBytes, elapsedMs))
        );
        return;
    }

    if (m_remoteStatusLabel != nullptr) {
        m_remoteStatusLabel->setText(statusPrefix() + QStringLiteral("upload failed: ") + result.message);
    }

    QMessageBox::warning(
        this,
        QStringLiteral("Upload failed — DD-SSH"),
        QStringLiteral("Could not upload local file:\n%1\n\nTarget:\n%2\n\n%3\n\nError:\n%4")
            .arg(localFilePath, remoteTargetPath, result.message, result.error)
    );
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

        QTableWidgetItem *sizeItem = new SizeTableWidgetItem(entry.sizeBytes);

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
