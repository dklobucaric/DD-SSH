#include "SftpBrowserTab.h"
#include "sftp/SftpProbe.h"
#include "core/AppLogger.h"

#include <QAbstractButton>
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

#include <algorithm>
#include <functional>
#include <utility>

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
    const QString &replacementDescription
)
{
    QMessageBox box(parent);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle(title);
    box.setText(QStringLiteral("Queue target already exists:\n%1\n\nOverwrite it with %2?").arg(targetPath, replacementDescription));
    box.setInformativeText(QStringLiteral("Choose one item, all remaining queue conflicts of this direction, or stop the queue."));

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

    m_localUploadButton = new QPushButton(QStringLiteral("Upload selected now"), localPanel);
    m_localUploadButton->setToolTip(QStringLiteral("Upload the first selected local file into the currently open remote folder immediately."));
    localPathLayout->addWidget(m_localUploadButton);

    m_localQueueUploadButton = new QPushButton(QStringLiteral("Queue upload(s)"), localPanel);
    m_localQueueUploadButton->setToolTip(QStringLiteral("Add selected local files to the transfer queue. Folder upload is intentionally not implemented yet."));
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

    m_remoteDownloadButton = new QPushButton(QStringLiteral("Download selected now"), remotePanel);
    m_remoteDownloadButton->setToolTip(QStringLiteral("Download the first selected remote file into the currently open local folder immediately."));
    remotePathLayout->addWidget(m_remoteDownloadButton);

    m_remoteQueueDownloadButton = new QPushButton(QStringLiteral("Queue download(s)"), remotePanel);
    m_remoteQueueDownloadButton->setToolTip(QStringLiteral("Add selected remote files to the transfer queue. Folder download is intentionally not implemented yet."));
    remotePathLayout->addWidget(m_remoteQueueDownloadButton);

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

    auto *queueTitle = new QLabel(QStringLiteral("Transfer queue (foundation — one file at a time)"), this);
    queueTitle->setToolTip(QStringLiteral("This checkpoint can queue multiple individual files and run them sequentially. Folder transfer, parallel transfer, resume, and sync are intentionally not implemented yet."));
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
        QStringLiteral("Single-file download/upload still works. Queue foundation adds multiple individual files, runs them one by one, and can retry selected finished items. Folder transfer, parallel transfer, resume, delete, rename, and sync are intentionally disabled."),
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


void SftpBrowserTab::queueSelectedRemoteDownloads()
{
    if (m_remoteTable == nullptr || m_remoteTable->selectionModel() == nullptr) {
        return;
    }

    const QModelIndexList selectedRows = m_remoteTable->selectionModel()->selectedRows(0);

    if (selectedRows.isEmpty()) {
        QMessageBox::information(
            this,
            QStringLiteral("No remote files selected — DD-SSH"),
            QStringLiteral("Select one or more remote files first, then click Queue download(s).")
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

    int added = 0;
    int skipped = 0;

    for (const QModelIndex &index : selectedRows) {
        const int row = index.row();
        QTableWidgetItem *nameItem = m_remoteTable->item(row, 0);
        QTableWidgetItem *typeItem = m_remoteTable->item(row, 1);
        QTableWidgetItem *sizeItem = m_remoteTable->item(row, 2);

        if (nameItem == nullptr || typeItem == nullptr) {
            ++skipped;
            continue;
        }

        const QString name = nameItem->data(Qt::UserRole).toString();
        const QString type = typeItem->data(Qt::UserRole).toString();

        if (name.trimmed().isEmpty() || name == QStringLiteral(".") || name == QStringLiteral("..") || isDirectoryType(type)) {
            ++skipped;
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
        item.status = QStringLiteral("Pending");
        item.message = QStringLiteral("Queued for download");
        m_transferQueue.append(item);
        ++added;
    }

    refreshTransferQueueTable();

    if (m_queueStatusLabel != nullptr) {
        m_queueStatusLabel->setText(QStringLiteral("Queue: added %1 download item(s), skipped %2 folder/unsupported item(s). %3")
            .arg(added)
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
            QStringLiteral("No local files selected — DD-SSH"),
            QStringLiteral("Select one or more local files first, then click Queue upload(s).")
        );
        return;
    }

    int added = 0;
    int skipped = 0;

    for (const QModelIndex &index : selectedRows) {
        if (!index.isValid()) {
            ++skipped;
            continue;
        }

        const QString localFilePath = m_localModel->filePath(index);
        const QFileInfo localInfo(localFilePath);

        if (!localInfo.exists() || localInfo.isDir() || !localInfo.isFile()) {
            ++skipped;
            continue;
        }

        TransferQueueItem item;
        item.direction = QStringLiteral("Upload");
        item.displayName = localInfo.fileName();
        item.sourcePath = localInfo.absoluteFilePath();
        item.targetPath = joinedRemotePath(m_currentRemotePath, localInfo.fileName());
        item.sizeBytes = static_cast<quint64>(localInfo.size());
        item.status = QStringLiteral("Pending");
        item.message = QStringLiteral("Queued for upload");
        m_transferQueue.append(item);
        ++added;
    }

    refreshTransferQueueTable();

    if (m_queueStatusLabel != nullptr) {
        m_queueStatusLabel->setText(QStringLiteral("Queue: added %1 upload item(s), skipped %2 folder/unsupported item(s). %3")
            .arg(added)
            .arg(skipped)
            .arg(transferQueueSummaryText()));
    }
}

void SftpBrowserTab::startTransferQueue()
{
    if (m_transferQueueRunning) {
        return;
    }

    int pendingCount = 0;
    for (const TransferQueueItem &item : std::as_const(m_transferQueue)) {
        if (item.status == QStringLiteral("Pending")) {
            ++pendingCount;
        }
    }

    if (pendingCount == 0) {
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

    m_transferQueueRunning = true;
    setTransferQueueBusy(true);

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

        if (item.direction == QStringLiteral("Download")) {
            const QFileInfo targetInfo(item.targetPath);
            const QFileInfo targetFolderInfo(targetInfo.absolutePath());

            if (!targetFolderInfo.exists() || !targetFolderInfo.isDir()) {
                setQueueItemStatus(i, QStringLiteral("Failed"), QStringLiteral("Local destination folder is not available"));
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
                        QStringLiteral("the remote file")
                    );

                    if (overwriteDecision == QueueOverwriteDecision::CancelQueue) {
                        setQueueItemStatus(i, QStringLiteral("Cancelled"), QStringLiteral("Queue stopped before overwrite"));
                        ++cancelledCount;
                        shouldStopQueue = true;
                        break;
                    }

                    if (overwriteDecision == QueueOverwriteDecision::SkipOne) {
                        setQueueItemStatus(i, QStringLiteral("Skipped"), QStringLiteral("Skipped existing local file"));
                        ++skippedCount;
                        continue;
                    }

                    if (overwriteDecision == QueueOverwriteDecision::SkipAll) {
                        skipAllExistingDownloads = true;
                        setQueueItemStatus(i, QStringLiteral("Skipped"), QStringLiteral("Skipped existing local file (skip all)"));
                        ++skippedCount;
                        continue;
                    }

                    if (overwriteDecision == QueueOverwriteDecision::OverwriteAll) {
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
                setQueueItemStatus(i, QStringLiteral("Done"), QStringLiteral("Downloaded %1 (%2), %3, %4")
                    .arg(formatSize(result.bytesTransferred), formatRawBytes(result.bytesTransferred), formatDuration(elapsedMs), formatTransferRate(result.bytesTransferred, elapsedMs)));
                ++doneCount;
                refreshLocalDirectory();
            } else if (result.cancelled || progressWasCancelled) {
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
                setQueueItemStatus(i, QStringLiteral("Failed"), result.message + QStringLiteral(" — ") + result.error);
                ++failedCount;
            }
        } else if (item.direction == QStringLiteral("Upload")) {
            const QFileInfo localInfo(item.sourcePath);

            if (!localInfo.exists() || !localInfo.isFile()) {
                setQueueItemStatus(i, QStringLiteral("Failed"), QStringLiteral("Local source file is not available"));
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
                        QStringLiteral("the local file")
                    );

                    if (overwriteDecision == QueueOverwriteDecision::CancelQueue) {
                        setQueueItemStatus(i, QStringLiteral("Cancelled"), QStringLiteral("Queue stopped before remote overwrite"));
                        ++cancelledCount;
                        shouldStopQueue = true;
                        break;
                    }

                    if (overwriteDecision == QueueOverwriteDecision::SkipOne) {
                        setQueueItemStatus(i, QStringLiteral("Skipped"), QStringLiteral("Skipped existing remote file"));
                        ++skippedCount;
                        break;
                    }

                    if (overwriteDecision == QueueOverwriteDecision::SkipAll) {
                        skipAllExistingUploads = true;
                        setQueueItemStatus(i, QStringLiteral("Skipped"), QStringLiteral("Skipped existing remote file (skip all)"));
                        ++skippedCount;
                        break;
                    }

                    if (overwriteDecision == QueueOverwriteDecision::OverwriteAll) {
                        overwriteAllUploads = true;
                    }

                    allowOverwrite = true;
                    uploadRetriedAfterOverwriteApproval = true;
                    continue;
                }

                if (result.success) {
                    setQueueItemStatus(i, QStringLiteral("Done"), QStringLiteral("Uploaded %1 (%2), %3, %4")
                        .arg(formatSize(result.bytesTransferred), formatRawBytes(result.bytesTransferred), formatDuration(elapsedMs), formatTransferRate(result.bytesTransferred, elapsedMs)));
                    ++doneCount;
                    refreshRemoteDirectory();
                } else if (result.cancelled || progressWasCancelled) {
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

    if (m_queueStatusLabel != nullptr) {
        m_queueStatusLabel->setText(QStringLiteral("Queue finished: done %1, failed %2, cancelled %3, skipped %4. %5")
            .arg(doneCount)
            .arg(failedCount)
            .arg(cancelledCount)
            .arg(skippedCount)
            .arg(transferQueueSummaryText()));
    }

    QMessageBox::information(
        this,
        QStringLiteral("Transfer queue finished — DD-SSH"),
        QStringLiteral("Transfer queue finished.\n\nDone: %1\nFailed: %2\nCancelled: %3\nSkipped: %4")
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
