#include "SftpBrowserTab.h"
#include "sftp/SftpProbe.h"
#include "core/AppLogger.h"

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QApplication>
#include <QDir>
#include <QDateTime>
#include <QDirIterator>
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

#include <algorithm>
#include <functional>
#include <utility>

namespace {
constexpr int kMaxFolderScanDepth = 64;
constexpr int kMaxQueuedFolderItems = 5000;
constexpr int kFolderQueueWarningItemThreshold = 250;

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

QString logSafeValue(QString value)
{
    value.replace(QLatin1Char('\r'), QLatin1Char(' '));
    value.replace(QLatin1Char('\n'), QLatin1Char(' '));
    value.replace(QLatin1Char('\t'), QLatin1Char(' '));

    if (value.size() > 512) {
        value = value.left(509) + QStringLiteral("...");
    }

    return value;
}

QString quotedLogValue(const QString &value)
{
    return QStringLiteral("\"") + logSafeValue(value) + QStringLiteral("\"");
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

QString formatSizeWithRawBytes(quint64 bytes)
{
    return QStringLiteral("%1 (%2)").arg(formatSize(bytes), formatRawBytes(bytes));
}

QString cleanMetadataText(const QString &value)
{
    const QString trimmed = value.trimmed();
    return trimmed.isEmpty() ? QStringLiteral("(unknown)") : trimmed;
}

QString formatLocalModifiedTime(const QFileInfo &info)
{
    if (!info.exists()) {
        return QStringLiteral("(unknown)");
    }

    const QDateTime modified = info.lastModified();
    return modified.isValid() ? modified.toString(Qt::ISODate) : QStringLiteral("(unknown)");
}

QString metadataBlock(
    const QString &title,
    const QString &path,
    bool sizeKnown,
    quint64 sizeBytes,
    const QString &modifiedTime
)
{
    QStringList lines;
    lines << title;
    lines << QStringLiteral("Path: %1").arg(path);
    lines << QStringLiteral("Size: %1").arg(sizeKnown ? formatSizeWithRawBytes(sizeBytes) : QStringLiteral("(unknown)"));
    lines << QStringLiteral("Modified: %1").arg(cleanMetadataText(modifiedTime));
    return lines.join(QLatin1Char('\n'));
}

QString overwriteMetadataMessage(
    const QString &intro,
    const QString &existingDetails,
    const QString &incomingDetails,
    const QString &question
)
{
    QStringList lines;
    lines << intro;
    lines << QString();
    lines << existingDetails;
    lines << QString();
    lines << incomingDetails;
    lines << QString();
    lines << question;
    return lines.join(QLatin1Char('\n'));
}


enum class QueueOverwriteDecision
{
    OverwriteOne,
    SkipOne,
    OverwriteAll,
    SkipAll,
    CancelQueue
};

QueueOverwriteDecision askQueueOverwriteDecision(
    QWidget *parent,
    const QString &title,
    const QString &targetPath,
    const QString &existingDetails,
    const QString &incomingDetails
)
{
    QMessageBox box(parent);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle(title);
    box.setText(overwriteMetadataMessage(
        QStringLiteral("Queue target already exists:"),
        existingDetails,
        incomingDetails,
        QStringLiteral("Overwrite this queue target?")));
    box.setInformativeText(QStringLiteral("Target: %1\nChoose one item, all remaining queue conflicts of this direction, or stop the queue.").arg(targetPath));

    QAbstractButton *overwriteButton = box.addButton(QStringLiteral("Overwrite"), QMessageBox::AcceptRole);
    QAbstractButton *skipButton = box.addButton(QStringLiteral("Skip"), QMessageBox::NoRole);
    QAbstractButton *overwriteAllButton = box.addButton(QStringLiteral("Overwrite all"), QMessageBox::AcceptRole);
    QAbstractButton *skipAllButton = box.addButton(QStringLiteral("Skip all"), QMessageBox::NoRole);
    QAbstractButton *cancelButton = box.addButton(QStringLiteral("Cancel queue"), QMessageBox::RejectRole);

    box.setDefaultButton(qobject_cast<QPushButton *>(overwriteButton));
    box.setEscapeButton(qobject_cast<QPushButton *>(cancelButton));
    box.exec();

    QAbstractButton *clicked = box.clickedButton();

    if (clicked == overwriteButton) {
        return QueueOverwriteDecision::OverwriteOne;
    }

    if (clicked == skipButton) {
        return QueueOverwriteDecision::SkipOne;
    }

    if (clicked == overwriteAllButton) {
        return QueueOverwriteDecision::OverwriteAll;
    }

    if (clicked == skipAllButton) {
        return QueueOverwriteDecision::SkipAll;
    }

    return QueueOverwriteDecision::CancelQueue;
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

bool SftpBrowserTab::hasTransferQueueWorkForExit() const
{
    if (m_transferQueueRunning) {
        return true;
    }

    for (const TransferQueueItem &item : m_transferQueue) {
        if (item.status == QStringLiteral("Pending") || item.status == QStringLiteral("Running")) {
            return true;
        }
    }

    return false;
}

QString SftpBrowserTab::transferQueueExitSummary() const
{
    const QString prefix = displayName();

    if (m_transferQueue.isEmpty()) {
        return prefix + QStringLiteral(": queue empty");
    }

    QString state = m_transferQueueRunning
        ? QStringLiteral("RUNNING")
        : QStringLiteral("not running");

    return prefix + QStringLiteral(": ") + state + QStringLiteral(", ") + transferQueueSummaryText();
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
    headerLabel->setToolTip(QStringLiteral("This checkpoint supports single-file download/upload, queue retries, experimental recursive folder queueing, and a conservative remote delete queue experiment. Rename, sync, resume, and parallel transfer actions are intentionally not implemented yet."));
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

    m_localUploadButton = new QPushButton(QStringLiteral("Upload selected now"), localPanel);
    m_localUploadButton->setToolTip(QStringLiteral("Upload the first selected local file into the currently open remote folder immediately."));
    localPathLayout->addWidget(m_localUploadButton);

    m_localQueueUploadButton = new QPushButton(QStringLiteral("Queue upload"), localPanel);
    m_localQueueUploadButton->setToolTip(QStringLiteral("Add selected local file(s) and folder(s) to the transfer queue. Folders are scanned recursively after confirmation; symlinks are skipped."));
    localPathLayout->addWidget(m_localQueueUploadButton);

    localLayout->addLayout(localPathLayout);

    m_localModel = new QFileSystemModel(this);
    m_localModel->setReadOnly(true);
    m_localModel->setFilter(QDir::AllEntries | QDir::AllDirs | QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot);

    m_localTree = new QTreeView(localPanel);
    m_localTree->setModel(m_localModel);
    m_localTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
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

    auto *remoteTitle = new QLabel(QStringLiteral("Remote files (SFTP — download/upload/delete queue)"), remotePanel);
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

    m_remoteDownloadButton = new QPushButton(QStringLiteral("Download selected now"), remotePanel);
    m_remoteDownloadButton->setToolTip(QStringLiteral("Download the first selected remote file into the currently open local folder immediately."));
    remotePathLayout->addWidget(m_remoteDownloadButton);

    m_remoteQueueDownloadButton = new QPushButton(QStringLiteral("Queue download"), remotePanel);
    m_remoteQueueDownloadButton->setToolTip(QStringLiteral("Add selected remote file(s) and folder(s) to the transfer queue. Folders are scanned recursively after confirmation; symlinks are skipped."));
    remotePathLayout->addWidget(m_remoteQueueDownloadButton);

    m_remoteQueueDeleteButton = new QPushButton(QStringLiteral("Queue delete"), remotePanel);
    m_remoteQueueDeleteButton->setToolTip(QStringLiteral("Add selected remote file(s) or empty folder(s) to the queue for deletion. A destructive confirmation is required before the queue runs delete items. Non-empty recursive folder delete is not implemented."));
    remotePathLayout->addWidget(m_remoteQueueDeleteButton);

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
    m_remoteTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
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

    auto *queueTitle = new QLabel(QStringLiteral("Transfer queue (folder experiment — one item at a time)"), this);
    queueTitle->setToolTip(QStringLiteral("This checkpoint can queue files and experimental recursive folder scans. Folder contents are expanded into normal queue items, summarized after scanning, and run sequentially."));
    mainLayout->addWidget(queueTitle);

    m_queueTable = new QTableWidget(this);
    m_queueTable->setColumnCount(6);
    m_queueTable->setHorizontalHeaderLabels(QStringList{
        QStringLiteral("Status"),
        QStringLiteral("Direction"),
        QStringLiteral("Name"),
        QStringLiteral("Size"),
        QStringLiteral("Source"),
        QStringLiteral("Target")
    });
    m_queueTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_queueTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_queueTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_queueTable->setAlternatingRowColors(false);
    m_queueTable->verticalHeader()->setVisible(false);
    m_queueTable->horizontalHeader()->setStretchLastSection(false);
    m_queueTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_queueTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_queueTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_queueTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_queueTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    m_queueTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    m_queueTable->setMaximumHeight(170);
    mainLayout->addWidget(m_queueTable);

    auto *queueControlsLayout = new QHBoxLayout();
    m_queueStartButton = new QPushButton(QStringLiteral("Start queue"), this);
    m_queueRetrySelectedButton = new QPushButton(QStringLiteral("Retry selected"), this);
    m_queueRetrySelectedButton->setToolTip(QStringLiteral("Move selected Done, Failed, Cancelled, or Skipped queue items back to Pending so Start queue can run them again."));
    m_queueRemoveSelectedButton = new QPushButton(QStringLiteral("Remove selected"), this);
    m_queueClearFinishedButton = new QPushButton(QStringLiteral("Clear finished"), this);
    m_queueStatusLabel = new QLabel(QStringLiteral("Queue: empty"), this);
    queueControlsLayout->addWidget(m_queueStartButton);
    queueControlsLayout->addWidget(m_queueRetrySelectedButton);
    queueControlsLayout->addWidget(m_queueRemoveSelectedButton);
    queueControlsLayout->addWidget(m_queueClearFinishedButton);
    queueControlsLayout->addWidget(m_queueStatusLabel, 1);
    mainLayout->addLayout(queueControlsLayout);

    auto *transferNotice = new QLabel(
        QStringLiteral("Single-file download/upload still works. Folder transfer is experimental: selected folders are scanned recursively, summarized, and expanded into queue items. Remote Queue delete is experimental and destructive: regular files, symlinks, and empty directories only; recursive folder delete is intentionally disabled. Resume, parallel transfer, rename, chmod, timestamps, and sync are intentionally disabled."),
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

    connect(m_localQueueUploadButton, &QPushButton::clicked, this, [this]() {
        queueSelectedLocalUploads();
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

    connect(m_remoteQueueDownloadButton, &QPushButton::clicked, this, [this]() {
        queueSelectedRemoteDownloads();
    });

    connect(m_remoteQueueDeleteButton, &QPushButton::clicked, this, [this]() {
        queueSelectedRemoteDeletes();
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

    connect(m_queueStartButton, &QPushButton::clicked, this, [this]() {
        startTransferQueue();
    });

    connect(m_queueRetrySelectedButton, &QPushButton::clicked, this, [this]() {
        retrySelectedTransferQueueItems();
    });

    connect(m_queueRemoveSelectedButton, &QPushButton::clicked, this, [this]() {
        removeSelectedTransferQueueItems();
    });

    connect(m_queueClearFinishedButton, &QPushButton::clicked, this, [this]() {
        clearFinishedTransferQueueItems();
    });

    refreshTransferQueueTable();
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

    if (m_remoteQueueDownloadButton != nullptr) {
        m_remoteQueueDownloadButton->setEnabled(!busy && !m_transferQueueRunning);
    }

    if (m_localQueueUploadButton != nullptr) {
        m_localQueueUploadButton->setEnabled(!busy && !m_transferQueueRunning);
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
    QTableWidgetItem *sizeItem = m_remoteTable->item(row, 2);
    QTableWidgetItem *modifiedItem = m_remoteTable->item(row, 3);

    if (nameItem == nullptr || typeItem == nullptr) {
        return;
    }

    const QString name = nameItem->data(Qt::UserRole).toString();
    const QString type = typeItem->data(Qt::UserRole).toString();
    const quint64 remoteSizeBytes = sizeItem != nullptr ? static_cast<quint64>(sizeItem->data(Qt::UserRole).toULongLong()) : 0ULL;
    const QString remoteModifiedTime = modifiedItem != nullptr ? modifiedItem->text() : QStringLiteral("(unknown)");

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
            QStringLiteral("Use Queue download — DD-SSH"),
            QStringLiteral("Immediate Download selected now handles one regular file. To download folders recursively, select them and use Queue download.")
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
        const QFileInfo existingLocalInfo(localTargetPath);
        const QString message = overwriteMetadataMessage(
            QStringLiteral("The local file already exists."),
            metadataBlock(
                QStringLiteral("Existing local file"),
                localTargetPath,
                true,
                static_cast<quint64>(existingLocalInfo.size()),
                formatLocalModifiedTime(existingLocalInfo)),
            metadataBlock(
                QStringLiteral("Incoming remote file"),
                remoteFilePath,
                true,
                remoteSizeBytes,
                remoteModifiedTime),
            QStringLiteral("Overwrite the local file with the selected remote file?")
        );

        const QMessageBox::StandardButton overwriteDecision = QMessageBox::warning(
            this,
            QStringLiteral("Overwrite local file? — DD-SSH"),
            message,
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
        AppLogger::info(QStringLiteral("SFTP immediate download completed: session=") + quotedLogValue(m_sessionName)
            + QStringLiteral(", remotePath=") + quotedLogValue(remoteFilePath)
            + QStringLiteral(", localPath=") + quotedLogValue(localTargetPath)
            + QStringLiteral(", bytes=") + QString::number(result.bytesTransferred)
            + QStringLiteral(", elapsedMs=") + QString::number(elapsedMs));
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
        AppLogger::warn(QStringLiteral("SFTP immediate download cancelled: session=") + quotedLogValue(m_sessionName)
            + QStringLiteral(", remotePath=") + quotedLogValue(remoteFilePath)
            + QStringLiteral(", localPath=") + quotedLogValue(localTargetPath)
            + QStringLiteral(", bytes=") + QString::number(result.bytesTransferred)
            + QStringLiteral(", elapsedMs=") + QString::number(elapsedMs));
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

    AppLogger::error(QStringLiteral("SFTP immediate download failed: session=") + quotedLogValue(m_sessionName)
        + QStringLiteral(", remotePath=") + quotedLogValue(remoteFilePath)
        + QStringLiteral(", localPath=") + quotedLogValue(localTargetPath)
        + QStringLiteral(", message=") + logSafeValue(result.message)
        + QStringLiteral(", error=") + logSafeValue(result.error));

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
            QStringLiteral("Use Queue upload — DD-SSH"),
            QStringLiteral("Immediate Upload selected now handles one regular file. To upload folders recursively, select them and use Queue upload.")
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
    quint64 remoteExistingSizeBytes = 0;
    QString remoteExistingModifiedTime = QStringLiteral("(unknown)");

    if (m_remoteTable != nullptr) {
        for (int row = 0; row < m_remoteTable->rowCount(); ++row) {
            QTableWidgetItem *nameItem = m_remoteTable->item(row, 0);
            QTableWidgetItem *typeItem = m_remoteTable->item(row, 1);
            QTableWidgetItem *sizeItem = m_remoteTable->item(row, 2);
            QTableWidgetItem *modifiedItem = m_remoteTable->item(row, 3);

            if (nameItem == nullptr || typeItem == nullptr) {
                continue;
            }

            const QString remoteName = nameItem->data(Qt::UserRole).toString();

            if (remoteName == fileName) {
                remoteExistsInCurrentListing = true;
                remoteTargetIsDirectory = isDirectoryType(typeItem->data(Qt::UserRole).toString());
                remoteExistingSizeBytes = sizeItem != nullptr ? static_cast<quint64>(sizeItem->data(Qt::UserRole).toULongLong()) : 0ULL;
                remoteExistingModifiedTime = modifiedItem != nullptr ? modifiedItem->text() : QStringLiteral("(unknown)");
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

        const QString message = overwriteMetadataMessage(
            QStringLiteral("The remote file already exists."),
            metadataBlock(
                QStringLiteral("Existing remote file"),
                remoteTargetPath,
                true,
                remoteExistingSizeBytes,
                remoteExistingModifiedTime),
            metadataBlock(
                QStringLiteral("Incoming local file"),
                localFilePath,
                true,
                static_cast<quint64>(localInfo.size()),
                formatLocalModifiedTime(localInfo)),
            QStringLiteral("Overwrite the remote file with the selected local file?")
        );

        const QMessageBox::StandardButton overwriteDecision = QMessageBox::warning(
            this,
            QStringLiteral("Overwrite remote file? — DD-SSH"),
            message,
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
        AppLogger::info(QStringLiteral("SFTP immediate upload completed: session=") + quotedLogValue(m_sessionName)
            + QStringLiteral(", localPath=") + quotedLogValue(localFilePath)
            + QStringLiteral(", remotePath=") + quotedLogValue(remoteTargetPath)
            + QStringLiteral(", bytes=") + QString::number(result.bytesTransferred)
            + QStringLiteral(", elapsedMs=") + QString::number(elapsedMs));
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
        AppLogger::warn(QStringLiteral("SFTP immediate upload cancelled: session=") + quotedLogValue(m_sessionName)
            + QStringLiteral(", localPath=") + quotedLogValue(localFilePath)
            + QStringLiteral(", remotePath=") + quotedLogValue(remoteTargetPath)
            + QStringLiteral(", bytes=") + QString::number(result.bytesTransferred)
            + QStringLiteral(", elapsedMs=") + QString::number(elapsedMs));
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

    AppLogger::error(QStringLiteral("SFTP immediate upload failed: session=") + quotedLogValue(m_sessionName)
        + QStringLiteral(", localPath=") + quotedLogValue(localFilePath)
        + QStringLiteral(", remotePath=") + quotedLogValue(remoteTargetPath)
        + QStringLiteral(", message=") + logSafeValue(result.message)
        + QStringLiteral(", error=") + logSafeValue(result.error));

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


void SftpBrowserTab::queueSelectedRemoteDownloads()
{
    if (m_remoteTable == nullptr || m_remoteTable->selectionModel() == nullptr) {
        return;
    }

    const QModelIndexList selectedRows = m_remoteTable->selectionModel()->selectedRows(0);

    if (selectedRows.isEmpty()) {
        QMessageBox::information(
            this,
            QStringLiteral("No remote items selected — DD-SSH"),
            QStringLiteral("Select one or more remote files or folders first, then click Queue download.")
        );
        return;
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

    int fileItemsAdded = 0;
    int folderFileItemsAdded = 0;
    int folderDirItemsAdded = 0;
    int skipped = 0;
    bool folderScanIssue = false;

    for (const QModelIndex &index : selectedRows) {
        const int row = index.row();
        QTableWidgetItem *nameItem = m_remoteTable->item(row, 0);
        QTableWidgetItem *typeItem = m_remoteTable->item(row, 1);
        QTableWidgetItem *sizeItem = m_remoteTable->item(row, 2);
        QTableWidgetItem *modifiedItem = m_remoteTable->item(row, 3);

        if (nameItem == nullptr || typeItem == nullptr) {
            ++skipped;
            continue;
        }

        const QString name = nameItem->data(Qt::UserRole).toString();
        const QString type = typeItem->data(Qt::UserRole).toString();

        if (name.trimmed().isEmpty() || name == QStringLiteral(".") || name == QStringLiteral("..")) {
            ++skipped;
            continue;
        }

        if (isDirectoryType(type)) {
            const QString remoteFolderPath = joinedRemotePath(m_currentRemotePath, name);
            const QString localTargetFolder = QDir(m_currentLocalPath).filePath(name);

            const bool confirmed = confirmFolderQueue(
                QStringLiteral("Queue remote folder download? — DD-SSH"),
                remoteFolderPath,
                QDir::cleanPath(localTargetFolder)
            );

            if (!confirmed) {
                ++skipped;
                continue;
            }

            QApplication::setOverrideCursor(Qt::WaitCursor);
            const bool scanOk = addRemoteFolderDownloadToQueue(remoteFolderPath, localTargetFolder, &folderFileItemsAdded, &folderDirItemsAdded, &skipped);
            QApplication::restoreOverrideCursor();
            QApplication::processEvents();

            if (!scanOk) {
                folderScanIssue = true;
            }

            continue;
        }

        if (type.compare(QStringLiteral("file"), Qt::CaseInsensitive) != 0
            && type.compare(QStringLiteral("symlink"), Qt::CaseInsensitive) != 0) {
            ++skipped;
            continue;
        }

        TransferQueueItem item;
        item.direction = QStringLiteral("Download");
        item.displayName = name;
        item.sourcePath = joinedRemotePath(m_currentRemotePath, name);
        item.targetPath = QDir(m_currentLocalPath).filePath(QFileInfo(name).fileName());
        item.sizeBytes = sizeItem != nullptr ? static_cast<quint64>(sizeItem->data(Qt::UserRole).toULongLong()) : 0ULL;
        item.sourceModifiedTime = modifiedItem != nullptr ? modifiedItem->text() : QStringLiteral("(unknown)");
        item.status = QStringLiteral("Pending");
        item.message = QStringLiteral("Queued for download");
        m_transferQueue.append(item);
        ++fileItemsAdded;
    }

    refreshTransferQueueTable();

    if (m_queueStatusLabel != nullptr) {
        m_queueStatusLabel->setText(QStringLiteral("Queue: added %1 direct download file item(s), %2 folder file item(s), %3 folder item(s), skipped %4 unsupported/cancelled item(s). %5")
            .arg(fileItemsAdded)
            .arg(folderFileItemsAdded)
            .arg(folderDirItemsAdded)
            .arg(skipped)
            .arg(transferQueueSummaryText()));
    }

    AppLogger::info(QStringLiteral("SFTP queue download selection processed: session=") + quotedLogValue(m_sessionName)
        + QStringLiteral(", directFiles=") + QString::number(fileItemsAdded)
        + QStringLiteral(", folderFiles=") + QString::number(folderFileItemsAdded)
        + QStringLiteral(", folderDirs=") + QString::number(folderDirItemsAdded)
        + QStringLiteral(", skipped=") + QString::number(skipped)
        + QStringLiteral(", scanIssue=") + (folderScanIssue ? QStringLiteral("true") : QStringLiteral("false")));


    const int folderItemsAdded = folderFileItemsAdded + folderDirItemsAdded;
    if (folderItemsAdded > 0 || skipped > 0 || folderScanIssue) {
        QStringList lines;
        lines << QStringLiteral("Folder queue scan complete.");
        lines << QString();
        lines << QStringLiteral("Direction: download");
        lines << QStringLiteral("Direct file item(s): %1").arg(fileItemsAdded);
        lines << QStringLiteral("Folder file item(s): %1").arg(folderFileItemsAdded);
        lines << QStringLiteral("Folder create item(s): %1").arg(folderDirItemsAdded);
        lines << QStringLiteral("Skipped/unsupported/cancelled item(s): %1").arg(skipped);

        if (folderItemsAdded >= kFolderQueueWarningItemThreshold) {
            lines << QString();
            lines << QStringLiteral("Large folder queue warning: review the queue before starting.");
        }

        if (folderScanIssue) {
            lines << QString();
            lines << QStringLiteral("One or more folder scans stopped early because of a permission/listing error, depth limit, or safety item limit.");
        }

        lines << QString();
        lines << QStringLiteral("The queue was not started automatically. Review it, then click Start queue.");

        QMessageBox::information(
            this,
            QStringLiteral("Folder queue summary — DD-SSH"),
            lines.join(QLatin1Char('\n'))
        );
    }
}


void SftpBrowserTab::queueSelectedRemoteDeletes()
{
    if (m_remoteTable == nullptr || m_remoteTable->selectionModel() == nullptr) {
        return;
    }

    const QModelIndexList selectedRows = m_remoteTable->selectionModel()->selectedRows(0);

    if (selectedRows.isEmpty()) {
        QMessageBox::information(
            this,
            QStringLiteral("No remote items selected — DD-SSH"),
            QStringLiteral("Select one or more remote files or empty folders first, then click Queue delete.")
        );
        return;
    }

    int fileItems = 0;
    int folderItems = 0;
    int skipped = 0;
    QList<TransferQueueItem> pendingDeleteItems;

    for (const QModelIndex &index : selectedRows) {
        const int row = index.row();
        QTableWidgetItem *nameItem = m_remoteTable->item(row, 0);
        QTableWidgetItem *typeItem = m_remoteTable->item(row, 1);
        QTableWidgetItem *sizeItem = m_remoteTable->item(row, 2);
        QTableWidgetItem *modifiedItem = m_remoteTable->item(row, 3);

        if (nameItem == nullptr || typeItem == nullptr) {
            ++skipped;
            continue;
        }

        const QString name = nameItem->data(Qt::UserRole).toString();
        const QString type = typeItem->data(Qt::UserRole).toString();

        if (name.trimmed().isEmpty() || name == QStringLiteral(".") || name == QStringLiteral("..")) {
            ++skipped;
            continue;
        }

        if (type.compare(QStringLiteral("file"), Qt::CaseInsensitive) != 0
            && type.compare(QStringLiteral("symlink"), Qt::CaseInsensitive) != 0
            && !isDirectoryType(type)) {
            ++skipped;
            continue;
        }

        TransferQueueItem item;
        item.direction = isDirectoryType(type) ? QStringLiteral("Delete remote dir") : QStringLiteral("Delete remote file");
        item.displayName = name;
        item.sourcePath = joinedRemotePath(m_currentRemotePath, name);
        item.targetPath = item.sourcePath;
        item.sizeBytes = sizeItem != nullptr ? static_cast<quint64>(sizeItem->data(Qt::UserRole).toULongLong()) : 0ULL;
        item.sourceModifiedTime = modifiedItem != nullptr ? modifiedItem->text() : QStringLiteral("(unknown)");
        item.status = QStringLiteral("Pending");
        item.message = isDirectoryType(type)
            ? QStringLiteral("Queued to delete remote empty folder")
            : QStringLiteral("Queued to delete remote file");
        pendingDeleteItems.append(item);

        if (isDirectoryType(type)) {
            ++folderItems;
        } else {
            ++fileItems;
        }
    }

    if (pendingDeleteItems.isEmpty()) {
        QMessageBox::information(
            this,
            QStringLiteral("Nothing queued for delete — DD-SSH"),
            QStringLiteral("No supported remote delete items were selected. Regular files, symlinks, and empty folders are supported in this checkpoint. Recursive non-empty folder delete is not implemented.")
        );
        return;
    }

    if (!confirmQueueRemoteDelete(fileItems, folderItems, skipped)) {
        AppLogger::warn(QStringLiteral("SFTP remote delete queue cancelled before enqueue: session=") + quotedLogValue(m_sessionName)
            + QStringLiteral(", files=") + QString::number(fileItems)
            + QStringLiteral(", folders=") + QString::number(folderItems)
            + QStringLiteral(", skipped=") + QString::number(skipped));
        return;
    }

    for (const TransferQueueItem &item : std::as_const(pendingDeleteItems)) {
        m_transferQueue.append(item);
        AppLogger::warn(QStringLiteral("SFTP remote delete queued: session=") + quotedLogValue(m_sessionName)
            + QStringLiteral(", path=") + quotedLogValue(item.targetPath)
            + QStringLiteral(", direction=") + logSafeValue(item.direction));
    }

    refreshTransferQueueTable();

    if (m_queueStatusLabel != nullptr) {
        m_queueStatusLabel->setText(QStringLiteral("Queue: added %1 remote delete file item(s), %2 remote delete empty-folder item(s), skipped %3 unsupported item(s). %4")
            .arg(fileItems)
            .arg(folderItems)
            .arg(skipped)
            .arg(transferQueueSummaryText()));
    }
}

void SftpBrowserTab::queueSelectedLocalUploads()
{
    if (m_localTree == nullptr || m_localTree->selectionModel() == nullptr || m_localModel == nullptr) {
        return;
    }

    const QModelIndexList selectedRows = m_localTree->selectionModel()->selectedRows(0);

    if (selectedRows.isEmpty()) {
        QMessageBox::information(
            this,
            QStringLiteral("No local items selected — DD-SSH"),
            QStringLiteral("Select one or more local files or folders first, then click Queue upload.")
        );
        return;
    }

    int fileItemsAdded = 0;
    int folderFileItemsAdded = 0;
    int folderDirItemsAdded = 0;
    int skipped = 0;
    bool folderScanIssue = false;

    for (const QModelIndex &index : selectedRows) {
        if (!index.isValid()) {
            ++skipped;
            continue;
        }

        const QString localPath = m_localModel->filePath(index);
        const QFileInfo localInfo(localPath);

        if (!localInfo.exists() || localInfo.isSymLink()) {
            ++skipped;
            continue;
        }

        if (localInfo.isDir()) {
            const QString remoteTargetFolder = joinedRemotePath(m_currentRemotePath, localInfo.fileName());

            const bool confirmed = confirmFolderQueue(
                QStringLiteral("Queue local folder upload? — DD-SSH"),
                localInfo.absoluteFilePath(),
                remoteTargetFolder
            );

            if (!confirmed) {
                ++skipped;
                continue;
            }

            QApplication::setOverrideCursor(Qt::WaitCursor);
            const bool scanOk = addLocalFolderUploadToQueue(localInfo.absoluteFilePath(), remoteTargetFolder, &folderFileItemsAdded, &folderDirItemsAdded, &skipped);
            QApplication::restoreOverrideCursor();
            QApplication::processEvents();

            if (!scanOk) {
                folderScanIssue = true;
            }

            continue;
        }

        if (!localInfo.isFile()) {
            ++skipped;
            continue;
        }

        TransferQueueItem item;
        item.direction = QStringLiteral("Upload");
        item.displayName = localInfo.fileName();
        item.sourcePath = localInfo.absoluteFilePath();
        item.targetPath = joinedRemotePath(m_currentRemotePath, localInfo.fileName());
        item.sizeBytes = static_cast<quint64>(localInfo.size());
        item.sourceModifiedTime = formatLocalModifiedTime(localInfo);
        item.status = QStringLiteral("Pending");
        item.message = QStringLiteral("Queued for upload");
        m_transferQueue.append(item);
        ++fileItemsAdded;
    }

    refreshTransferQueueTable();

    if (m_queueStatusLabel != nullptr) {
        m_queueStatusLabel->setText(QStringLiteral("Queue: added %1 direct upload file item(s), %2 folder file item(s), %3 folder item(s), skipped %4 unsupported/cancelled item(s). %5")
            .arg(fileItemsAdded)
            .arg(folderFileItemsAdded)
            .arg(folderDirItemsAdded)
            .arg(skipped)
            .arg(transferQueueSummaryText()));
    }

    AppLogger::info(QStringLiteral("SFTP queue upload selection processed: session=") + quotedLogValue(m_sessionName)
        + QStringLiteral(", directFiles=") + QString::number(fileItemsAdded)
        + QStringLiteral(", folderFiles=") + QString::number(folderFileItemsAdded)
        + QStringLiteral(", folderDirs=") + QString::number(folderDirItemsAdded)
        + QStringLiteral(", skipped=") + QString::number(skipped)
        + QStringLiteral(", scanIssue=") + (folderScanIssue ? QStringLiteral("true") : QStringLiteral("false")));

    const int folderItemsAdded = folderFileItemsAdded + folderDirItemsAdded;
    if (folderItemsAdded > 0 || skipped > 0 || folderScanIssue) {
        QStringList lines;
        lines << QStringLiteral("Folder queue scan complete.");
        lines << QString();
        lines << QStringLiteral("Direction: upload");
        lines << QStringLiteral("Direct file item(s): %1").arg(fileItemsAdded);
        lines << QStringLiteral("Folder file item(s): %1").arg(folderFileItemsAdded);
        lines << QStringLiteral("Folder create item(s): %1").arg(folderDirItemsAdded);
        lines << QStringLiteral("Skipped/unsupported/cancelled item(s): %1").arg(skipped);

        if (folderItemsAdded >= kFolderQueueWarningItemThreshold) {
            lines << QString();
            lines << QStringLiteral("Large folder queue warning: review the queue before starting.");
        }

        if (folderScanIssue) {
            lines << QString();
            lines << QStringLiteral("One or more folder scans stopped early because of a permission/listing error, depth limit, or safety item limit.");
        }

        lines << QString();
        lines << QStringLiteral("The queue was not started automatically. Review it, then click Start queue.");

        QMessageBox::information(
            this,
            QStringLiteral("Folder queue summary — DD-SSH"),
            lines.join(QLatin1Char('\n'))
        );
    }
}

bool SftpBrowserTab::confirmFolderQueue(const QString &title, const QString &sourcePath, const QString &targetPath) const
{
    const QMessageBox::StandardButton decision = QMessageBox::question(
        const_cast<SftpBrowserTab *>(this),
        title,
        QStringLiteral("Queue folder recursively?\n\nSource:\n%1\n\nTarget:\n%2\n\nThis may add many files to the transfer queue. DD-SSH will scan the folder first, expand supported files/folders into normal queue items, then wait for you to click Start queue.\n\nSafety rules in this checkpoint:\n- symlinks and special files are skipped\n- folder permissions and timestamps are not preserved\n- resume, sync, mirror, and parallel transfer are not implemented\n- safety limit: up to %3 scanned folder queue items")
            .arg(sourcePath, targetPath)
            .arg(kMaxQueuedFolderItems),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    AppLogger::info(QStringLiteral("SFTP folder queue confirmation: session=") + quotedLogValue(m_sessionName)
        + QStringLiteral(", accepted=") + (decision == QMessageBox::Yes ? QStringLiteral("true") : QStringLiteral("false"))
        + QStringLiteral(", source=") + quotedLogValue(sourcePath)
        + QStringLiteral(", target=") + quotedLogValue(targetPath));

    return decision == QMessageBox::Yes;
}


bool SftpBrowserTab::confirmQueueRemoteDelete(int fileCount, int folderCount, int skippedCount) const
{
    QStringList lines;
    lines << QStringLiteral("Add selected remote item(s) to the delete queue?");
    lines << QString();
    lines << QStringLiteral("Files/symlinks: %1").arg(fileCount);
    lines << QStringLiteral("Folders: %1").arg(folderCount);

    if (skippedCount > 0) {
        lines << QStringLiteral("Skipped unsupported item(s): %1").arg(skippedCount);
    }

    lines << QString();
    lines << QStringLiteral("Delete is destructive and happens on the remote server. There is no recycle bin.");
    lines << QStringLiteral("This checkpoint supports regular files, symlinks, and empty folders only.");
    lines << QStringLiteral("Recursive non-empty folder delete is intentionally not implemented yet.");
    lines << QString();
    lines << QStringLiteral("The queue will ask again before running pending delete item(s).");

    const QMessageBox::StandardButton decision = QMessageBox::warning(
        const_cast<SftpBrowserTab *>(this),
        QStringLiteral("Queue remote delete? — DD-SSH"),
        lines.join(QLatin1Char('\n')),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    AppLogger::warn(QStringLiteral("SFTP remote delete enqueue confirmation: session=") + quotedLogValue(m_sessionName)
        + QStringLiteral(", accepted=") + (decision == QMessageBox::Yes ? QStringLiteral("true") : QStringLiteral("false"))
        + QStringLiteral(", files=") + QString::number(fileCount)
        + QStringLiteral(", folders=") + QString::number(folderCount)
        + QStringLiteral(", skipped=") + QString::number(skippedCount));

    return decision == QMessageBox::Yes;
}

bool SftpBrowserTab::confirmPendingRemoteDeleteRun(int deleteFileCount, int deleteDirCount) const
{
    if (deleteFileCount <= 0 && deleteDirCount <= 0) {
        return true;
    }

    QStringList lines;
    lines << QStringLiteral("You have queued remote item(s) for deletion.");
    lines << QString();
    lines << QStringLiteral("Remote files/symlinks: %1").arg(deleteFileCount);
    lines << QStringLiteral("Remote folders: %1").arg(deleteDirCount);
    lines << QString();
    lines << QStringLiteral("This will permanently delete remote item(s) from the server.");
    lines << QStringLiteral("This is not a recycle bin operation.");
    lines << QStringLiteral("Non-empty recursive folder delete is not supported; non-empty folders should fail safely.");
    lines << QString();
    lines << QStringLiteral("Continue with the queue?");

    const QMessageBox::StandardButton decision = QMessageBox::warning(
        const_cast<SftpBrowserTab *>(this),
        QStringLiteral("Confirm remote delete queue — DD-SSH"),
        lines.join(QLatin1Char('\n')),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    AppLogger::warn(QStringLiteral("SFTP remote delete run confirmation: session=") + quotedLogValue(m_sessionName)
        + QStringLiteral(", accepted=") + (decision == QMessageBox::Yes ? QStringLiteral("true") : QStringLiteral("false"))
        + QStringLiteral(", files=") + QString::number(deleteFileCount)
        + QStringLiteral(", folders=") + QString::number(deleteDirCount));

    return decision == QMessageBox::Yes;
}

bool SftpBrowserTab::addRemoteFolderDownloadToQueue(
    const QString &remoteFolderPath,
    const QString &localTargetFolder,
    int *filesAdded,
    int *dirsAdded,
    int *skipped,
    int depth
)
{
    if (filesAdded == nullptr || dirsAdded == nullptr || skipped == nullptr) {
        return false;
    }

    if (depth > kMaxFolderScanDepth) {
        ++(*skipped);
        return false;
    }

    if ((*filesAdded + *dirsAdded) >= kMaxQueuedFolderItems) {
        ++(*skipped);
        return false;
    }

    TransferQueueItem directoryItem;
    directoryItem.direction = QStringLiteral("Create local dir");
    directoryItem.displayName = QFileInfo(localTargetFolder).fileName().isEmpty() ? localTargetFolder : QFileInfo(localTargetFolder).fileName();
    directoryItem.sourcePath = remoteFolderPath;
    directoryItem.targetPath = QDir::cleanPath(localTargetFolder);
    directoryItem.status = QStringLiteral("Pending");
    directoryItem.message = QStringLiteral("Queued to create local folder");
    m_transferQueue.append(directoryItem);
    ++(*dirsAdded);

    const SftpProbeResult listing = SftpProbe::listRemoteDirectory(
        m_host,
        m_port,
        m_username,
        m_authMethod,
        m_secretValue,
        m_hostKeyExpectation,
        remoteFolderPath
    );

    if (!listing.success) {
        TransferQueueItem failedMarker;
        failedMarker.direction = QStringLiteral("Create local dir");
        failedMarker.displayName = QStringLiteral("Scan failed: ") + QFileInfo(localTargetFolder).fileName();
        failedMarker.sourcePath = remoteFolderPath;
        failedMarker.targetPath = QDir::cleanPath(localTargetFolder);
        failedMarker.status = QStringLiteral("Failed");
        failedMarker.message = listing.message + QStringLiteral(" — ") + listing.error;
        m_transferQueue.append(failedMarker);
        ++(*skipped);
        return false;
    }

    for (const SftpRemoteEntry &entry : listing.entries) {
        if ((*filesAdded + *dirsAdded) >= kMaxQueuedFolderItems) {
            ++(*skipped);
            return false;
        }

        if (entry.name.trimmed().isEmpty() || entry.name == QStringLiteral(".") || entry.name == QStringLiteral("..")) {
            continue;
        }

        const QString childRemotePath = joinedRemotePath(remoteFolderPath, entry.name);
        const QString childLocalPath = QDir(localTargetFolder).filePath(entry.name);

        if (isDirectoryType(entry.type)) {
            const bool childScanOk = addRemoteFolderDownloadToQueue(childRemotePath, childLocalPath, filesAdded, dirsAdded, skipped, depth + 1);
            if (!childScanOk) {
                return false;
            }
            continue;
        }

        if (entry.type.compare(QStringLiteral("file"), Qt::CaseInsensitive) != 0) {
            ++(*skipped);
            continue;
        }

        TransferQueueItem fileItem;
        fileItem.direction = QStringLiteral("Download");
        fileItem.displayName = entry.name;
        fileItem.sourcePath = childRemotePath;
        fileItem.targetPath = QDir::cleanPath(childLocalPath);
        fileItem.sizeBytes = entry.sizeBytes;
        fileItem.sourceModifiedTime = entry.modifiedTime;
        fileItem.status = QStringLiteral("Pending");
        fileItem.message = QStringLiteral("Queued by folder download scan");
        m_transferQueue.append(fileItem);
        ++(*filesAdded);
    }

    return true;
}

bool SftpBrowserTab::addLocalFolderUploadToQueue(
    const QString &localFolderPath,
    const QString &remoteTargetFolder,
    int *filesAdded,
    int *dirsAdded,
    int *skipped
)
{
    if (filesAdded == nullptr || dirsAdded == nullptr || skipped == nullptr) {
        return false;
    }

    const QFileInfo rootInfo(localFolderPath);
    if (!rootInfo.exists() || !rootInfo.isDir() || rootInfo.isSymLink()) {
        ++(*skipped);
        return false;
    }

    TransferQueueItem rootDirItem;
    rootDirItem.direction = QStringLiteral("Create remote dir");
    rootDirItem.displayName = rootInfo.fileName();
    rootDirItem.sourcePath = rootInfo.absoluteFilePath();
    rootDirItem.targetPath = remoteTargetFolder;
    rootDirItem.status = QStringLiteral("Pending");
    rootDirItem.message = QStringLiteral("Queued to create remote folder");
    m_transferQueue.append(rootDirItem);
    ++(*dirsAdded);

    const QDir rootDir(rootInfo.absoluteFilePath());
    QDirIterator iterator(
        rootInfo.absoluteFilePath(),
        QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot,
        QDirIterator::Subdirectories
    );

    while (iterator.hasNext()) {
        iterator.next();

        if ((*filesAdded + *dirsAdded) >= kMaxQueuedFolderItems) {
            ++(*skipped);
            return false;
        }

        const QFileInfo info = iterator.fileInfo();

        if (info.isSymLink()) {
            ++(*skipped);
            continue;
        }

        QString relativePath = rootDir.relativeFilePath(info.absoluteFilePath());
        relativePath.replace(QDir::separator(), QLatin1Char('/'));
        const QString remotePath = joinedRemotePath(remoteTargetFolder, relativePath);

        if (info.isDir()) {
            TransferQueueItem dirItem;
            dirItem.direction = QStringLiteral("Create remote dir");
            dirItem.displayName = info.fileName();
            dirItem.sourcePath = info.absoluteFilePath();
            dirItem.targetPath = remotePath;
            dirItem.status = QStringLiteral("Pending");
            dirItem.message = QStringLiteral("Queued to create remote folder");
            m_transferQueue.append(dirItem);
            ++(*dirsAdded);
            continue;
        }

        if (!info.isFile()) {
            ++(*skipped);
            continue;
        }

        TransferQueueItem fileItem;
        fileItem.direction = QStringLiteral("Upload");
        fileItem.displayName = relativePath;
        fileItem.sourcePath = info.absoluteFilePath();
        fileItem.targetPath = remotePath;
        fileItem.sizeBytes = static_cast<quint64>(info.size());
        fileItem.sourceModifiedTime = formatLocalModifiedTime(info);
        fileItem.status = QStringLiteral("Pending");
        fileItem.message = QStringLiteral("Queued by folder upload scan");
        m_transferQueue.append(fileItem);
        ++(*filesAdded);
    }

    return true;
}

void SftpBrowserTab::startTransferQueue()
{
    if (m_transferQueueRunning) {
        return;
    }

    int pendingCount = 0;
    int pendingDeleteFileCount = 0;
    int pendingDeleteDirCount = 0;

    for (const TransferQueueItem &item : std::as_const(m_transferQueue)) {
        if (item.status == QStringLiteral("Pending")) {
            ++pendingCount;

            if (item.direction == QStringLiteral("Delete remote file")) {
                ++pendingDeleteFileCount;
            } else if (item.direction == QStringLiteral("Delete remote dir")) {
                ++pendingDeleteDirCount;
            }
        }
    }

    if (pendingCount == 0) {
        AppLogger::info(QStringLiteral("SFTP transfer queue start ignored: session=") + quotedLogValue(m_sessionName)
            + QStringLiteral(", reason=no pending items"));
        if (m_queueStatusLabel != nullptr) {
            m_queueStatusLabel->setText(QStringLiteral("Queue: no pending items. Use Retry selected or add new files before starting."));
        }

        QMessageBox::information(
            this,
            QStringLiteral("No pending queue items — DD-SSH"),
            QStringLiteral("There are no pending transfer items in the queue.\n\nAdd files to the queue or use Retry selected on finished/skipped/cancelled items first.")
        );
        return;
    }

    if (!confirmPendingRemoteDeleteRun(pendingDeleteFileCount, pendingDeleteDirCount)) {
        AppLogger::warn(QStringLiteral("SFTP transfer queue start cancelled by remote delete confirmation: session=") + quotedLogValue(m_sessionName)
            + QStringLiteral(", pendingDeleteFiles=") + QString::number(pendingDeleteFileCount)
            + QStringLiteral(", pendingDeleteDirs=") + QString::number(pendingDeleteDirCount));

        if (m_queueStatusLabel != nullptr) {
            m_queueStatusLabel->setText(QStringLiteral("Queue: start cancelled because pending remote delete item(s) were not confirmed. %1").arg(transferQueueSummaryText()));
        }

        return;
    }

    m_transferQueueRunning = true;
    setTransferQueueBusy(true);

    AppLogger::info(QStringLiteral("SFTP transfer queue started: session=") + quotedLogValue(m_sessionName)
        + QStringLiteral(", pending=") + QString::number(pendingCount)
        + QStringLiteral(", totalItems=") + QString::number(m_transferQueue.size()));

    int doneCount = 0;
    int failedCount = 0;
    int cancelledCount = 0;
    int skippedCount = 0;
    bool shouldStopQueue = false;
    bool overwriteAllDownloads = false;
    bool skipAllExistingDownloads = false;
    bool overwriteAllUploads = false;
    bool skipAllExistingUploads = false;

    for (int i = 0; i < m_transferQueue.size(); ++i) {
        if (shouldStopQueue) {
            break;
        }

        TransferQueueItem &item = m_transferQueue[i];

        if (item.status != QStringLiteral("Pending")) {
            continue;
        }

        setQueueItemStatus(i, QStringLiteral("Running"), QStringLiteral("Checking target"));
        AppLogger::info(QStringLiteral("SFTP queue item started: session=") + quotedLogValue(m_sessionName)
            + QStringLiteral(", index=") + QString::number(i + 1)
            + QStringLiteral(", direction=") + logSafeValue(item.direction)
            + QStringLiteral(", name=") + quotedLogValue(item.displayName)
            + QStringLiteral(", source=") + quotedLogValue(item.sourcePath)
            + QStringLiteral(", target=") + quotedLogValue(item.targetPath)
            + QStringLiteral(", sizeBytes=") + QString::number(item.sizeBytes));

        if (item.direction == QStringLiteral("Create local dir")) {
            const QFileInfo existing(item.targetPath);

            if (existing.exists() && !existing.isDir()) {
                setQueueItemStatus(i, QStringLiteral("Failed"), QStringLiteral("Local target exists and is not a folder: ") + item.targetPath);
                ++failedCount;
                continue;
            }

            if (QDir().mkpath(item.targetPath)) {
                setQueueItemStatus(i, QStringLiteral("Done"), existing.exists() ? QStringLiteral("Local folder already exists") : QStringLiteral("Local folder ready"));
                AppLogger::info(QStringLiteral("SFTP queue create local dir completed: session=") + quotedLogValue(m_sessionName)
                    + QStringLiteral(", target=") + quotedLogValue(item.targetPath)
                    + QStringLiteral(", alreadyExisted=") + (existing.exists() ? QStringLiteral("true") : QStringLiteral("false")));
                ++doneCount;
            } else {
                setQueueItemStatus(i, QStringLiteral("Failed"), QStringLiteral("Could not create local folder: ") + item.targetPath);
                AppLogger::error(QStringLiteral("SFTP queue create local dir failed: session=") + quotedLogValue(m_sessionName)
                    + QStringLiteral(", target=") + quotedLogValue(item.targetPath));
                ++failedCount;
            }

            continue;
        }

        if (item.direction == QStringLiteral("Create remote dir")) {
            const SftpMkdirResult mkdirResult = SftpProbe::createRemoteDirectory(
                m_host,
                m_port,
                m_username,
                m_authMethod,
                m_secretValue,
                m_hostKeyExpectation,
                item.targetPath
            );

            if (mkdirResult.success) {
                setQueueItemStatus(i, QStringLiteral("Done"), mkdirResult.alreadyExists ? QStringLiteral("Remote folder already exists") : QStringLiteral("Remote folder created"));
                AppLogger::info(QStringLiteral("SFTP queue create remote dir completed: session=") + quotedLogValue(m_sessionName)
                    + QStringLiteral(", target=") + quotedLogValue(item.targetPath)
                    + QStringLiteral(", alreadyExisted=") + (mkdirResult.alreadyExists ? QStringLiteral("true") : QStringLiteral("false")));
                ++doneCount;
            } else {
                setQueueItemStatus(i, QStringLiteral("Failed"), mkdirResult.message + QStringLiteral(" — ") + mkdirResult.error);
                AppLogger::error(QStringLiteral("SFTP queue create remote dir failed: session=") + quotedLogValue(m_sessionName)
                    + QStringLiteral(", target=") + quotedLogValue(item.targetPath)
                    + QStringLiteral(", message=") + logSafeValue(mkdirResult.message)
                    + QStringLiteral(", error=") + logSafeValue(mkdirResult.error));
                ++failedCount;
            }

            continue;
        }


        if (item.direction == QStringLiteral("Delete remote file") || item.direction == QStringLiteral("Delete remote dir")) {
            setQueueItemStatus(i, QStringLiteral("Running"), QStringLiteral("Deleting remote item"));

            AppLogger::warn(QStringLiteral("SFTP queue remote delete started: session=") + quotedLogValue(m_sessionName)
                + QStringLiteral(", target=") + quotedLogValue(item.targetPath)
                + QStringLiteral(", direction=") + logSafeValue(item.direction));

            const SftpDeleteResult deleteResult = SftpProbe::deleteRemotePath(
                m_host,
                m_port,
                m_username,
                m_authMethod,
                m_secretValue,
                m_hostKeyExpectation,
                item.targetPath
            );

            if (deleteResult.success) {
                setQueueItemStatus(i, QStringLiteral("Done"), deleteResult.remotePathIsDirectory ? QStringLiteral("Deleted remote empty folder") : QStringLiteral("Deleted remote file"));
                AppLogger::warn(QStringLiteral("SFTP queue remote delete completed: session=") + quotedLogValue(m_sessionName)
                    + QStringLiteral(", target=") + quotedLogValue(item.targetPath)
                    + QStringLiteral(", directory=") + (deleteResult.remotePathIsDirectory ? QStringLiteral("true") : QStringLiteral("false")));
                ++doneCount;
                refreshRemoteDirectory();
            } else {
                setQueueItemStatus(i, QStringLiteral("Failed"), deleteResult.message + QStringLiteral(" — ") + deleteResult.error);
                AppLogger::error(QStringLiteral("SFTP queue remote delete failed: session=") + quotedLogValue(m_sessionName)
                    + QStringLiteral(", target=") + quotedLogValue(item.targetPath)
                    + QStringLiteral(", message=") + logSafeValue(deleteResult.message)
                    + QStringLiteral(", error=") + logSafeValue(deleteResult.error));
                ++failedCount;
            }

            continue;
        }

        if (item.direction == QStringLiteral("Download")) {
            const QFileInfo targetInfo(item.targetPath);
            const QFileInfo targetFolderInfo(targetInfo.absolutePath());

            if (!targetFolderInfo.exists() || !targetFolderInfo.isDir()) {
                setQueueItemStatus(i, QStringLiteral("Failed"), QStringLiteral("Local destination folder is not available: ") + targetFolderInfo.absoluteFilePath());
                ++failedCount;
                continue;
            }

            if (QFileInfo::exists(item.targetPath)) {
                if (skipAllExistingDownloads) {
                    setQueueItemStatus(i, QStringLiteral("Skipped"), QStringLiteral("Skipped existing local file (skip all)"));
                    ++skippedCount;
                    continue;
                }

                if (!overwriteAllDownloads) {
                    const QueueOverwriteDecision overwriteDecision = askQueueOverwriteDecision(
                        this,
                        QStringLiteral("Overwrite local file? — DD-SSH"),
                        item.targetPath,
                        metadataBlock(
                            QStringLiteral("Existing local file"),
                            item.targetPath,
                            true,
                            static_cast<quint64>(targetInfo.size()),
                            formatLocalModifiedTime(targetInfo)),
                        metadataBlock(
                            QStringLiteral("Incoming remote file"),
                            item.sourcePath,
                            true,
                            item.sizeBytes,
                            item.sourceModifiedTime)
                    );

                    if (overwriteDecision == QueueOverwriteDecision::CancelQueue) {
                        AppLogger::warn(QStringLiteral("SFTP queue download overwrite cancelled queue: session=") + quotedLogValue(m_sessionName)
                            + QStringLiteral(", target=") + quotedLogValue(item.targetPath));
                        setQueueItemStatus(i, QStringLiteral("Cancelled"), QStringLiteral("Queue stopped before overwrite"));
                        ++cancelledCount;
                        shouldStopQueue = true;
                        break;
                    }

                    if (overwriteDecision == QueueOverwriteDecision::SkipOne) {
                        AppLogger::warn(QStringLiteral("SFTP queue download skipped existing target: session=") + quotedLogValue(m_sessionName)
                            + QStringLiteral(", target=") + quotedLogValue(item.targetPath));
                        setQueueItemStatus(i, QStringLiteral("Skipped"), QStringLiteral("Skipped existing local file"));
                        ++skippedCount;
                        continue;
                    }

                    if (overwriteDecision == QueueOverwriteDecision::SkipAll) {
                        AppLogger::warn(QStringLiteral("SFTP queue download skip-all selected: session=") + quotedLogValue(m_sessionName));
                        skipAllExistingDownloads = true;
                        setQueueItemStatus(i, QStringLiteral("Skipped"), QStringLiteral("Skipped existing local file (skip all)"));
                        ++skippedCount;
                        continue;
                    }

                    if (overwriteDecision == QueueOverwriteDecision::OverwriteAll) {
                        AppLogger::info(QStringLiteral("SFTP queue download overwrite-all selected: session=") + quotedLogValue(m_sessionName));
                        overwriteAllDownloads = true;
                    }
                }
            }

            setQueueItemStatus(i, QStringLiteral("Running"), QStringLiteral("Transferring"));

            QElapsedTimer transferTimer;
            transferTimer.start();
            bool progressWasCancelled = false;

            QProgressDialog progress(
                QStringLiteral("Queue item %1/%2: download %3 ...")
                    .arg(i + 1)
                    .arg(m_transferQueue.size())
                    .arg(item.displayName),
                QStringLiteral("Cancel item"),
                0,
                100,
                this
            );
            progress.setWindowTitle(QStringLiteral("SFTP transfer queue — DD-SSH"));
            progress.setWindowModality(Qt::ApplicationModal);
            progress.setMinimumDuration(0);
            progress.setValue(0);
            progress.show();
            progress.raise();
            progress.activateWindow();
            QApplication::processEvents();

            const SftpDownloadResult result = SftpProbe::downloadRemoteFile(
                m_host,
                m_port,
                m_username,
                m_authMethod,
                m_secretValue,
                m_hostKeyExpectation,
                item.sourcePath,
                item.targetPath,
                [&progress, &progressWasCancelled, &transferTimer, &item](quint64 bytesTransferred, quint64 totalBytes, const QString &message) -> bool {
                    int percent = 0;

                    if (totalBytes > 0) {
                        percent = static_cast<int>(qMin<quint64>(100, (bytesTransferred * 100ULL) / totalBytes));
                    } else if (bytesTransferred > 0) {
                        percent = 50;
                    }

                    progress.setValue(percent);
                    progress.setLabelText(formatTransferProgressLabel(
                        QStringLiteral("Queue download"),
                        message + QStringLiteral(" — ") + item.displayName,
                        bytesTransferred,
                        totalBytes,
                        transferTimer.elapsed()
                    ));
                    QApplication::processEvents();

                    if (progress.wasCanceled()) {
                        progressWasCancelled = true;
                        return false;
                    }

                    return true;
                }
            );

            const qint64 elapsedMs = transferTimer.elapsed();
            progress.close();
            QApplication::processEvents();

            if (result.success) {
                AppLogger::info(QStringLiteral("SFTP queue download completed: session=") + quotedLogValue(m_sessionName)
                    + QStringLiteral(", source=") + quotedLogValue(item.sourcePath)
                    + QStringLiteral(", target=") + quotedLogValue(item.targetPath)
                    + QStringLiteral(", bytes=") + QString::number(result.bytesTransferred)
                    + QStringLiteral(", elapsedMs=") + QString::number(elapsedMs));
                setQueueItemStatus(i, QStringLiteral("Done"), QStringLiteral("Downloaded %1 (%2), %3, %4")
                    .arg(formatSize(result.bytesTransferred), formatRawBytes(result.bytesTransferred), formatDuration(elapsedMs), formatTransferRate(result.bytesTransferred, elapsedMs)));
                ++doneCount;
                refreshLocalDirectory();
            } else if (result.cancelled || progressWasCancelled) {
                AppLogger::warn(QStringLiteral("SFTP queue download cancelled: session=") + quotedLogValue(m_sessionName)
                    + QStringLiteral(", source=") + quotedLogValue(item.sourcePath)
                    + QStringLiteral(", target=") + quotedLogValue(item.targetPath)
                    + QStringLiteral(", bytes=") + QString::number(result.bytesTransferred));
                setQueueItemStatus(i, QStringLiteral("Cancelled"), QStringLiteral("Download cancelled; local target was not replaced"));
                ++cancelledCount;

                const QMessageBox::StandardButton continueDecision = QMessageBox::question(
                    this,
                    QStringLiteral("Queue item cancelled — DD-SSH"),
                    QStringLiteral("Download cancelled:\n%1\n\nLocal target was not replaced because DD-SSH uses a safe temporary download file.\n\nContinue with remaining queued items?").arg(item.displayName),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::No
                );

                if (continueDecision != QMessageBox::Yes) {
                    shouldStopQueue = true;
                }
            } else {
                AppLogger::error(QStringLiteral("SFTP queue download failed: session=") + quotedLogValue(m_sessionName)
                    + QStringLiteral(", source=") + quotedLogValue(item.sourcePath)
                    + QStringLiteral(", target=") + quotedLogValue(item.targetPath)
                    + QStringLiteral(", message=") + logSafeValue(result.message)
                    + QStringLiteral(", error=") + logSafeValue(result.error));
                setQueueItemStatus(i, QStringLiteral("Failed"), result.message + QStringLiteral(" — ") + result.error);
                ++failedCount;
            }
        } else if (item.direction == QStringLiteral("Upload")) {
            const QFileInfo localInfo(item.sourcePath);

            if (!localInfo.exists() || !localInfo.isFile()) {
                setQueueItemStatus(i, QStringLiteral("Failed"), QStringLiteral("Local source file is not available: ") + item.sourcePath);
                ++failedCount;
                continue;
            }

            bool allowOverwrite = overwriteAllUploads;
            bool uploadRetriedAfterOverwriteApproval = overwriteAllUploads;

            while (true) {
                setQueueItemStatus(i, QStringLiteral("Running"), allowOverwrite ? QStringLiteral("Transferring with overwrite") : QStringLiteral("Checking remote target"));

                QElapsedTimer transferTimer;
                transferTimer.start();
                bool progressWasCancelled = false;

                QProgressDialog progress(
                    QStringLiteral("Queue item %1/%2: upload %3 ...")
                        .arg(i + 1)
                        .arg(m_transferQueue.size())
                        .arg(item.displayName),
                    QStringLiteral("Cancel item"),
                    0,
                    100,
                    this
                );
                progress.setWindowTitle(QStringLiteral("SFTP transfer queue — DD-SSH"));
                progress.setWindowModality(Qt::ApplicationModal);
                progress.setMinimumDuration(0);
                progress.setValue(0);
                progress.show();
                progress.raise();
                progress.activateWindow();
                QApplication::processEvents();

                const SftpUploadResult result = SftpProbe::uploadLocalFile(
                    m_host,
                    m_port,
                    m_username,
                    m_authMethod,
                    m_secretValue,
                    m_hostKeyExpectation,
                    item.sourcePath,
                    item.targetPath,
                    allowOverwrite,
                    [&progress, &progressWasCancelled, &transferTimer, &item](quint64 bytesTransferred, quint64 totalBytes, const QString &message) -> bool {
                        int percent = 0;

                        if (totalBytes > 0) {
                            percent = static_cast<int>(qMin<quint64>(100, (bytesTransferred * 100ULL) / totalBytes));
                        } else if (bytesTransferred > 0) {
                            percent = 50;
                        }

                        progress.setValue(percent);
                        progress.setLabelText(formatTransferProgressLabel(
                            QStringLiteral("Queue upload"),
                            message + QStringLiteral(" — ") + item.displayName,
                            bytesTransferred,
                            totalBytes,
                            transferTimer.elapsed()
                        ));
                        QApplication::processEvents();

                        if (progress.wasCanceled()) {
                            progressWasCancelled = true;
                            return false;
                        }

                        return true;
                    }
                );

                const qint64 elapsedMs = transferTimer.elapsed();
                progress.close();
                QApplication::processEvents();

                if (result.remoteTargetIsDirectory) {
                    setQueueItemStatus(i, QStringLiteral("Failed"), QStringLiteral("Remote target is a directory"));
                    ++failedCount;
                    break;
                }

                if (result.remoteAlreadyExists && !allowOverwrite && !uploadRetriedAfterOverwriteApproval) {
                    if (skipAllExistingUploads) {
                        setQueueItemStatus(i, QStringLiteral("Skipped"), QStringLiteral("Skipped existing remote file (skip all)"));
                        ++skippedCount;
                        break;
                    }

                    const QueueOverwriteDecision overwriteDecision = askQueueOverwriteDecision(
                        this,
                        QStringLiteral("Overwrite remote file? — DD-SSH"),
                        item.targetPath,
                        metadataBlock(
                            QStringLiteral("Existing remote file"),
                            item.targetPath,
                            true,
                            result.remoteExistingSizeBytes,
                            result.remoteExistingModifiedTime),
                        metadataBlock(
                            QStringLiteral("Incoming local file"),
                            item.sourcePath,
                            true,
                            item.sizeBytes,
                            item.sourceModifiedTime)
                    );

                    if (overwriteDecision == QueueOverwriteDecision::CancelQueue) {
                        AppLogger::warn(QStringLiteral("SFTP queue upload overwrite cancelled queue: session=") + quotedLogValue(m_sessionName)
                            + QStringLiteral(", target=") + quotedLogValue(item.targetPath));
                        setQueueItemStatus(i, QStringLiteral("Cancelled"), QStringLiteral("Queue stopped before remote overwrite"));
                        ++cancelledCount;
                        shouldStopQueue = true;
                        break;
                    }

                    if (overwriteDecision == QueueOverwriteDecision::SkipOne) {
                        AppLogger::warn(QStringLiteral("SFTP queue upload skipped existing target: session=") + quotedLogValue(m_sessionName)
                            + QStringLiteral(", target=") + quotedLogValue(item.targetPath));
                        setQueueItemStatus(i, QStringLiteral("Skipped"), QStringLiteral("Skipped existing remote file"));
                        ++skippedCount;
                        break;
                    }

                    if (overwriteDecision == QueueOverwriteDecision::SkipAll) {
                        AppLogger::warn(QStringLiteral("SFTP queue upload skip-all selected: session=") + quotedLogValue(m_sessionName));
                        skipAllExistingUploads = true;
                        setQueueItemStatus(i, QStringLiteral("Skipped"), QStringLiteral("Skipped existing remote file (skip all)"));
                        ++skippedCount;
                        break;
                    }

                    if (overwriteDecision == QueueOverwriteDecision::OverwriteAll) {
                        AppLogger::info(QStringLiteral("SFTP queue upload overwrite-all selected: session=") + quotedLogValue(m_sessionName));
                        overwriteAllUploads = true;
                    }

                    allowOverwrite = true;
                    uploadRetriedAfterOverwriteApproval = true;
                    continue;
                }

                if (result.success) {
                    AppLogger::info(QStringLiteral("SFTP queue upload completed: session=") + quotedLogValue(m_sessionName)
                        + QStringLiteral(", source=") + quotedLogValue(item.sourcePath)
                        + QStringLiteral(", target=") + quotedLogValue(item.targetPath)
                        + QStringLiteral(", bytes=") + QString::number(result.bytesTransferred)
                        + QStringLiteral(", elapsedMs=") + QString::number(elapsedMs));
                    setQueueItemStatus(i, QStringLiteral("Done"), QStringLiteral("Uploaded %1 (%2), %3, %4")
                        .arg(formatSize(result.bytesTransferred), formatRawBytes(result.bytesTransferred), formatDuration(elapsedMs), formatTransferRate(result.bytesTransferred, elapsedMs)));
                    ++doneCount;
                    refreshRemoteDirectory();
                } else if (result.cancelled || progressWasCancelled) {
                    AppLogger::warn(QStringLiteral("SFTP queue upload cancelled: session=") + quotedLogValue(m_sessionName)
                        + QStringLiteral(", source=") + quotedLogValue(item.sourcePath)
                        + QStringLiteral(", target=") + quotedLogValue(item.targetPath)
                        + QStringLiteral(", bytes=") + QString::number(result.bytesTransferred));
                    setQueueItemStatus(i, QStringLiteral("Cancelled"), QStringLiteral("Upload cancelled; partial remote file may remain"));
                    ++cancelledCount;
                    refreshRemoteDirectory();

                    const QMessageBox::StandardButton continueDecision = QMessageBox::question(
                        this,
                        QStringLiteral("Queue item cancelled — DD-SSH"),
                        QStringLiteral("Upload cancelled:\n%1\n\nA partial remote file may remain on the server. Refresh/check the remote folder before retrying.\n\nContinue with remaining queued items?").arg(item.displayName),
                        QMessageBox::Yes | QMessageBox::No,
                        QMessageBox::No
                    );

                    if (continueDecision != QMessageBox::Yes) {
                        shouldStopQueue = true;
                    }
                } else {
                    AppLogger::error(QStringLiteral("SFTP queue upload failed: session=") + quotedLogValue(m_sessionName)
                        + QStringLiteral(", source=") + quotedLogValue(item.sourcePath)
                        + QStringLiteral(", target=") + quotedLogValue(item.targetPath)
                        + QStringLiteral(", message=") + logSafeValue(result.message)
                        + QStringLiteral(", error=") + logSafeValue(result.error));
                    setQueueItemStatus(i, QStringLiteral("Failed"), result.message + QStringLiteral(" — ") + result.error);
                    ++failedCount;
                }

                break;
            }
        } else {
            setQueueItemStatus(i, QStringLiteral("Failed"), QStringLiteral("Unknown transfer direction"));
            ++failedCount;
        }
    }

    m_transferQueueRunning = false;
    setTransferQueueBusy(false);
    refreshTransferQueueTable();
    refreshLocalDirectory();
    refreshRemoteDirectory();

    if (m_queueStatusLabel != nullptr) {
        m_queueStatusLabel->setText(QStringLiteral("Queue finished: done %1, failed %2, cancelled %3, skipped %4. %5")
            .arg(doneCount)
            .arg(failedCount)
            .arg(cancelledCount)
            .arg(skippedCount)
            .arg(transferQueueSummaryText()));
    }

    AppLogger::info(QStringLiteral("SFTP transfer queue finished: session=") + quotedLogValue(m_sessionName)
        + QStringLiteral(", done=") + QString::number(doneCount)
        + QStringLiteral(", failed=") + QString::number(failedCount)
        + QStringLiteral(", cancelled=") + QString::number(cancelledCount)
        + QStringLiteral(", skipped=") + QString::number(skippedCount)
        + QStringLiteral(", totalItems=") + QString::number(m_transferQueue.size()));

    QMessageBox::information(
        this,
        QStringLiteral("Transfer queue finished — DD-SSH"),
        QStringLiteral("Transfer queue finished.\n\nDone: %1\nFailed: %2\nCancelled: %3\nSkipped: %4\n\nLocal and remote panels were refreshed after the queue run.")
            .arg(doneCount)
            .arg(failedCount)
            .arg(cancelledCount)
            .arg(skippedCount)
    );
}

void SftpBrowserTab::clearFinishedTransferQueueItems()
{
    if (m_transferQueueRunning) {
        return;
    }

    QList<TransferQueueItem> kept;

    for (const TransferQueueItem &item : std::as_const(m_transferQueue)) {
        if (item.status == QStringLiteral("Pending") || item.status == QStringLiteral("Running")) {
            kept.append(item);
        }
    }

    m_transferQueue = kept;
    refreshTransferQueueTable();
}

void SftpBrowserTab::retrySelectedTransferQueueItems()
{
    if (m_transferQueueRunning || m_queueTable == nullptr || m_queueTable->selectionModel() == nullptr) {
        return;
    }

    const QModelIndexList selectedRows = m_queueTable->selectionModel()->selectedRows(0);

    if (selectedRows.isEmpty()) {
        QMessageBox::information(
            this,
            QStringLiteral("No queue items selected — DD-SSH"),
            QStringLiteral("Select one or more finished queue items first, then click Retry selected.")
        );
        return;
    }

    int requeued = 0;
    int alreadyPending = 0;
    int skippedRunning = 0;
    int notRetryable = 0;

    for (const QModelIndex &index : selectedRows) {
        const int row = index.row();

        if (row < 0 || row >= m_transferQueue.size()) {
            continue;
        }

        TransferQueueItem &item = m_transferQueue[row];

        if (item.status == QStringLiteral("Running")) {
            ++skippedRunning;
            continue;
        }

        if (item.status == QStringLiteral("Pending")) {
            ++alreadyPending;
            continue;
        }

        if (item.status == QStringLiteral("Done")
            || item.status == QStringLiteral("Failed")
            || item.status == QStringLiteral("Cancelled")
            || item.status == QStringLiteral("Skipped")) {
            item.status = QStringLiteral("Pending");
            item.message = QStringLiteral("Requeued for retry");
            ++requeued;
            continue;
        }

        ++notRetryable;
    }

    refreshTransferQueueTable();

    if (m_queueStatusLabel != nullptr) {
        m_queueStatusLabel->setText(QStringLiteral("Queue: requeued %1 item(s), already pending %2, running skipped %3, not retryable %4. %5")
            .arg(requeued)
            .arg(alreadyPending)
            .arg(skippedRunning)
            .arg(notRetryable)
            .arg(transferQueueSummaryText()));
    }

    AppLogger::info(QStringLiteral("SFTP transfer queue retry selected: session=") + quotedLogValue(m_sessionName)
        + QStringLiteral(", requeued=") + QString::number(requeued)
        + QStringLiteral(", alreadyPending=") + QString::number(alreadyPending)
        + QStringLiteral(", runningSkipped=") + QString::number(skippedRunning)
        + QStringLiteral(", notRetryable=") + QString::number(notRetryable));
}

void SftpBrowserTab::removeSelectedTransferQueueItems()
{
    if (m_transferQueueRunning || m_queueTable == nullptr || m_queueTable->selectionModel() == nullptr) {
        return;
    }

    const QModelIndexList selectedRows = m_queueTable->selectionModel()->selectedRows(0);

    if (selectedRows.isEmpty()) {
        return;
    }

    QList<int> rows;
    for (const QModelIndex &index : selectedRows) {
        rows.append(index.row());
    }
    std::sort(rows.begin(), rows.end(), std::greater<int>());

    for (int row : rows) {
        if (row >= 0 && row < m_transferQueue.size() && m_transferQueue[row].status != QStringLiteral("Running")) {
            m_transferQueue.removeAt(row);
        }
    }

    refreshTransferQueueTable();
}

void SftpBrowserTab::refreshTransferQueueTable()
{
    if (m_queueTable == nullptr) {
        return;
    }

    m_queueTable->setSortingEnabled(false);
    m_queueTable->clearContents();
    m_queueTable->setRowCount(m_transferQueue.size());

    for (int row = 0; row < m_transferQueue.size(); ++row) {
        const TransferQueueItem &item = m_transferQueue.at(row);

        auto *statusItem = makeReadOnlyItem(item.status);
        statusItem->setToolTip(item.message);

        auto *directionItem = makeReadOnlyItem(item.direction);
        auto *nameItem = makeReadOnlyItem(item.displayName);
        auto *sizeItem = new SizeTableWidgetItem(item.sizeBytes);
        auto *sourceItem = makeReadOnlyItem(item.sourcePath);
        sourceItem->setToolTip(item.sourcePath);
        auto *targetItem = makeReadOnlyItem(item.targetPath);
        targetItem->setToolTip(item.targetPath);

        m_queueTable->setItem(row, 0, statusItem);
        m_queueTable->setItem(row, 1, directionItem);
        m_queueTable->setItem(row, 2, nameItem);
        m_queueTable->setItem(row, 3, sizeItem);
        m_queueTable->setItem(row, 4, sourceItem);
        m_queueTable->setItem(row, 5, targetItem);
    }

    if (m_queueStatusLabel != nullptr && !m_transferQueueRunning) {
        m_queueStatusLabel->setText(QStringLiteral("Queue: ") + transferQueueSummaryText());
    }
}

void SftpBrowserTab::setTransferQueueBusy(bool busy)
{
    if (m_queueStartButton != nullptr) {
        m_queueStartButton->setEnabled(!busy);
    }

    if (m_queueRetrySelectedButton != nullptr) {
        m_queueRetrySelectedButton->setEnabled(!busy);
    }

    if (m_queueRemoveSelectedButton != nullptr) {
        m_queueRemoveSelectedButton->setEnabled(!busy);
    }

    if (m_queueClearFinishedButton != nullptr) {
        m_queueClearFinishedButton->setEnabled(!busy);
    }

    if (m_localGoButton != nullptr) {
        m_localGoButton->setEnabled(!busy);
    }

    if (m_localUpButton != nullptr) {
        m_localUpButton->setEnabled(!busy);
    }

    if (m_localRefreshButton != nullptr) {
        m_localRefreshButton->setEnabled(!busy);
    }

    if (m_localPathEdit != nullptr) {
        m_localPathEdit->setEnabled(!busy);
    }

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

    if (m_localUploadButton != nullptr) {
        m_localUploadButton->setEnabled(!busy);
    }

    if (m_localQueueUploadButton != nullptr) {
        m_localQueueUploadButton->setEnabled(!busy);
    }

    if (m_remoteDownloadButton != nullptr) {
        m_remoteDownloadButton->setEnabled(!busy);
    }

    if (m_remoteQueueDownloadButton != nullptr) {
        m_remoteQueueDownloadButton->setEnabled(!busy);
    }

    if (m_remoteQueueDeleteButton != nullptr) {
        m_remoteQueueDeleteButton->setEnabled(!busy);
    }

    if (m_localTree != nullptr) {
        m_localTree->setEnabled(!busy);
    }

    if (m_remoteTable != nullptr) {
        m_remoteTable->setEnabled(!busy);
    }

    if (m_queueTable != nullptr) {
        m_queueTable->setEnabled(!busy);
    }

    if (m_queueStatusLabel != nullptr && busy) {
        m_queueStatusLabel->setText(QStringLiteral("Queue: running ... navigation and queue editing are locked until the current run finishes."));
    }
}

void SftpBrowserTab::setQueueItemStatus(int index, const QString &status, const QString &message)
{
    if (index < 0 || index >= m_transferQueue.size()) {
        return;
    }

    m_transferQueue[index].status = status;
    m_transferQueue[index].message = message;
    refreshTransferQueueTable();
    QApplication::processEvents();
}

QString SftpBrowserTab::transferQueueSummaryText() const
{
    if (m_transferQueue.isEmpty()) {
        return QStringLiteral("empty");
    }

    int pending = 0;
    int running = 0;
    int done = 0;
    int failed = 0;
    int cancelled = 0;
    int skipped = 0;

    for (const TransferQueueItem &item : m_transferQueue) {
        if (item.status == QStringLiteral("Pending")) {
            ++pending;
        } else if (item.status == QStringLiteral("Running")) {
            ++running;
        } else if (item.status == QStringLiteral("Done")) {
            ++done;
        } else if (item.status == QStringLiteral("Failed")) {
            ++failed;
        } else if (item.status == QStringLiteral("Cancelled")) {
            ++cancelled;
        } else if (item.status == QStringLiteral("Skipped")) {
            ++skipped;
        }
    }

    return QStringLiteral("%1 item(s): pending %2, running %3, done %4, failed %5, cancelled %6, skipped %7")
        .arg(m_transferQueue.size())
        .arg(pending)
        .arg(running)
        .arg(done)
        .arg(failed)
        .arg(cancelled)
        .arg(skipped);
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
