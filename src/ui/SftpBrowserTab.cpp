#include "SftpBrowserTab.h"
#include "sftp/SftpProbe.h"
#include "core/AppLogger.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
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
{
    setupUi();
    refreshDirectory();
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
        QStringLiteral("Read-only SFTP browser — %1@%2:%3")
            .arg(m_username, m_host, QString::number(m_port)),
        this
    );
    headerLabel->setToolTip(QStringLiteral("This checkpoint can browse remote directories only. Upload, download, delete, rename, and queue actions are intentionally not implemented yet."));
    mainLayout->addWidget(headerLabel);

    auto *pathLayout = new QHBoxLayout();

    auto *pathLabel = new QLabel(QStringLiteral("Remote path:"), this);
    pathLayout->addWidget(pathLabel);

    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setText(m_currentPath);
    m_pathEdit->setPlaceholderText(QStringLiteral("., /home/user, /var/log ..."));
    pathLayout->addWidget(m_pathEdit, 1);

    m_goButton = new QPushButton(QStringLiteral("Go"), this);
    pathLayout->addWidget(m_goButton);

    m_upButton = new QPushButton(QStringLiteral("↑ Up"), this);
    pathLayout->addWidget(m_upButton);

    m_refreshButton = new QPushButton(QStringLiteral("Refresh"), this);
    pathLayout->addWidget(m_refreshButton);

    mainLayout->addLayout(pathLayout);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels(QStringList{
        QStringLiteral("Name"),
        QStringLiteral("Type"),
        QStringLiteral("Size"),
        QStringLiteral("Modified"),
        QStringLiteral("Permissions")
    });
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(false);
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSortingEnabled(true);
    mainLayout->addWidget(m_table, 1);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setText(QStringLiteral("Ready"));
    mainLayout->addWidget(m_statusLabel);

    connect(m_refreshButton, &QPushButton::clicked, this, [this]() {
        refreshDirectory();
    });

    connect(m_goButton, &QPushButton::clicked, this, [this]() {
        openPathFromEditor();
    });

    connect(m_pathEdit, &QLineEdit::returnPressed, this, [this]() {
        openPathFromEditor();
    });

    connect(m_upButton, &QPushButton::clicked, this, [this]() {
        goUp();
    });

    connect(m_table, &QTableWidget::cellDoubleClicked, this, [this](int row, int column) {
        handleCellDoubleClicked(row, column);
    });
}

void SftpBrowserTab::setBusy(bool busy)
{
    if (m_goButton != nullptr) {
        m_goButton->setEnabled(!busy);
    }

    if (m_upButton != nullptr) {
        m_upButton->setEnabled(!busy);
    }

    if (m_refreshButton != nullptr) {
        m_refreshButton->setEnabled(!busy);
    }

    if (m_pathEdit != nullptr) {
        m_pathEdit->setEnabled(!busy);
    }
}

QString SftpBrowserTab::normalizedPath(const QString &path) const
{
    const QString trimmed = path.trimmed();
    return trimmed.isEmpty() ? QStringLiteral(".") : trimmed;
}

QString SftpBrowserTab::joinedRemotePath(const QString &basePath, const QString &entryName) const
{
    const QString name = entryName.trimmed();

    if (name.isEmpty() || name == QStringLiteral(".")) {
        return normalizedPath(basePath);
    }

    if (name == QStringLiteral("..")) {
        return parentRemotePath(basePath);
    }

    QString base = normalizedPath(basePath);

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
    QString normalized = normalizedPath(path);

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

void SftpBrowserTab::refreshDirectory()
{
    const QString requestedPath = normalizedPath(m_currentPath);

    AppLogger::info(QStringLiteral("SFTP browser directory refresh requested: session=\"") + m_sessionName
        + QStringLiteral("\", host=") + m_host
        + QStringLiteral(", port=") + QString::number(m_port)
        + QStringLiteral(", path=") + requestedPath);

    setBusy(true);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (m_statusLabel != nullptr) {
        m_statusLabel->setText(statusPrefix() + QStringLiteral("loading ") + requestedPath + QStringLiteral(" ..."));
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
    setBusy(false);

    if (!result.success) {
        AppLogger::warn(QStringLiteral("SFTP browser directory refresh failed: session=\"") + m_sessionName
            + QStringLiteral("\", path=") + requestedPath
            + QStringLiteral(", message=") + result.message
            + QStringLiteral(", error=") + result.error);

        if (m_statusLabel != nullptr) {
            m_statusLabel->setText(statusPrefix() + QStringLiteral("failed to load ") + requestedPath + QStringLiteral(" — ") + result.message);
        }

        QMessageBox::warning(
            this,
            QStringLiteral("SFTP browser failed — DD-SSH"),
            QStringLiteral("Could not list remote path:\n%1\n\n%2\n\nError:\n%3")
                .arg(requestedPath, result.message, result.error)
        );
        return;
    }

    m_currentPath = result.remotePath;

    if (m_pathEdit != nullptr) {
        m_pathEdit->setText(m_currentPath);
    }

    populateTable(result.entries);

    if (m_statusLabel != nullptr) {
        m_statusLabel->setText(statusPrefix() + QStringLiteral("loaded ") + m_currentPath
            + QStringLiteral(" — ") + QString::number(result.entries.size()) + QStringLiteral(" entries (read-only)"));
    }

    AppLogger::info(QStringLiteral("SFTP browser directory refresh successful: session=\"") + m_sessionName
        + QStringLiteral("\", path=") + m_currentPath
        + QStringLiteral(", entries=") + QString::number(result.entries.size()));
}

void SftpBrowserTab::openPathFromEditor()
{
    if (m_pathEdit == nullptr) {
        return;
    }

    m_currentPath = normalizedPath(m_pathEdit->text());
    refreshDirectory();
}

void SftpBrowserTab::goUp()
{
    m_currentPath = parentRemotePath(m_currentPath);

    if (m_pathEdit != nullptr) {
        m_pathEdit->setText(m_currentPath);
    }

    refreshDirectory();
}

void SftpBrowserTab::handleCellDoubleClicked(int row, int)
{
    if (m_table == nullptr || row < 0 || row >= m_table->rowCount()) {
        return;
    }

    QTableWidgetItem *nameItem = m_table->item(row, 0);
    QTableWidgetItem *typeItem = m_table->item(row, 1);

    if (nameItem == nullptr || typeItem == nullptr) {
        return;
    }

    const QString name = nameItem->data(Qt::UserRole).toString();
    const QString type = typeItem->data(Qt::UserRole).toString();

    if (!isDirectoryType(type)) {
        if (m_statusLabel != nullptr) {
            m_statusLabel->setText(statusPrefix() + QStringLiteral("selected file ") + name + QStringLiteral(" — transfers are not implemented yet"));
        }
        return;
    }

    const QString nextPath = joinedRemotePath(m_currentPath, name);

    if (nextPath == m_currentPath && name == QStringLiteral(".")) {
        return;
    }

    m_currentPath = nextPath;

    if (m_pathEdit != nullptr) {
        m_pathEdit->setText(m_currentPath);
    }

    refreshDirectory();
}

void SftpBrowserTab::populateTable(const QList<SftpRemoteEntry> &entries)
{
    if (m_table == nullptr) {
        return;
    }

    m_table->setSortingEnabled(false);
    m_table->clearContents();
    m_table->setRowCount(entries.size());

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

        m_table->setItem(row, 0, nameItem);
        m_table->setItem(row, 1, typeItem);
        m_table->setItem(row, 2, sizeItem);
        m_table->setItem(row, 3, modifiedItem);
        m_table->setItem(row, 4, permissionsItem);
    }

    m_table->setSortingEnabled(true);
    m_table->sortItems(0, Qt::AscendingOrder);
}
