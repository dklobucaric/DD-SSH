#pragma once

#include "sftp/SftpProbe.h"

#include <QList>
#include <QString>
#include <QWidget>

class QFileSystemModel;
class QLabel;
class QLineEdit;
class QPushButton;
class QModelIndex;
class QTableWidget;
class QTreeView;

class SftpBrowserTab : public QWidget
{
public:
    SftpBrowserTab(
        const QString &sessionName,
        const QString &host,
        int port,
        const QString &username,
        SshAuthMethod authMethod,
        const QString &secretValue,
        const SshHostKeyExpectation &hostKeyExpectation,
        QWidget *parent = nullptr
    );

    QString displayName() const;
    bool hasTransferQueueWorkForExit() const;
    QString transferQueueExitSummary() const;

private:
    void setupUi();

    void openLocalPathFromEditor();
    void goLocalUp();
    void refreshLocalDirectory();
    void handleLocalDoubleClicked(const QModelIndex &index);
    void setLocalPath(const QString &path);
    QString normalizedLocalPath(const QString &path) const;

    void refreshRemoteDirectory();
    void openRemotePathFromEditor();
    void goRemoteUp();
    void handleRemoteCellDoubleClicked(int row, int column);
    void downloadSelectedRemoteFile();
    void uploadSelectedLocalFile();
    void queueSelectedRemoteDownloads();
    void queueSelectedRemoteDeletes();
    void queueSelectedLocalUploads();
    void startTransferQueue();
    void clearFinishedTransferQueueItems();
    void removeSelectedTransferQueueItems();
    void retrySelectedTransferQueueItems();
    void refreshTransferQueueTable();
    void setTransferQueueBusy(bool busy);
    void setQueueItemStatus(int index, const QString &status, const QString &message = QString());
    QString transferQueueSummaryText() const;
    bool confirmFolderQueue(const QString &title, const QString &sourcePath, const QString &targetPath) const;
    bool confirmQueueRemoteDelete(int fileCount, int folderCount, int skippedCount) const;
    bool confirmPendingRemoteDeleteRun(int deleteFileCount, int deleteDirCount) const;
    bool addRemoteFolderDownloadToQueue(const QString &remoteFolderPath, const QString &localTargetFolder, int *filesAdded, int *dirsAdded, int *skipped, int depth = 0);
    bool addLocalFolderUploadToQueue(const QString &localFolderPath, const QString &remoteTargetFolder, int *filesAdded, int *dirsAdded, int *skipped);
    void populateRemoteTable(const QList<SftpRemoteEntry> &entries);
    void setRemoteBusy(bool busy);
    QString normalizedRemotePath(const QString &path) const;
    QString joinedRemotePath(const QString &basePath, const QString &entryName) const;
    QString parentRemotePath(const QString &path) const;
    QString statusPrefix() const;

    QString m_sessionName;
    QString m_host;
    int m_port = 22;
    QString m_username;
    SshAuthMethod m_authMethod = SshAuthMethod::Password;
    QString m_secretValue;
    SshHostKeyExpectation m_hostKeyExpectation;

    QString m_currentLocalPath;
    QString m_currentRemotePath = QStringLiteral(".");

    struct TransferQueueItem
    {
        QString direction;
        QString displayName;
        QString sourcePath;
        QString targetPath;
        quint64 sizeBytes = 0;
        QString sourceModifiedTime;
        QString status = QStringLiteral("Pending");
        QString message;
    };

    QList<TransferQueueItem> m_transferQueue;
    bool m_transferQueueRunning = false;

    QLineEdit *m_localPathEdit = nullptr;
    QPushButton *m_localGoButton = nullptr;
    QPushButton *m_localUpButton = nullptr;
    QPushButton *m_localRefreshButton = nullptr;
    QPushButton *m_localUploadButton = nullptr;
    QPushButton *m_localQueueUploadButton = nullptr;
    QLabel *m_localStatusLabel = nullptr;
    QFileSystemModel *m_localModel = nullptr;
    QTreeView *m_localTree = nullptr;

    QLineEdit *m_remotePathEdit = nullptr;
    QPushButton *m_remoteGoButton = nullptr;
    QPushButton *m_remoteUpButton = nullptr;
    QPushButton *m_remoteRefreshButton = nullptr;
    QPushButton *m_remoteDownloadButton = nullptr;
    QPushButton *m_remoteQueueDownloadButton = nullptr;
    QPushButton *m_remoteQueueDeleteButton = nullptr;
    QLabel *m_remoteStatusLabel = nullptr;
    QTableWidget *m_remoteTable = nullptr;

    QTableWidget *m_queueTable = nullptr;
    QPushButton *m_queueStartButton = nullptr;
    QPushButton *m_queueRetrySelectedButton = nullptr;
    QPushButton *m_queueRemoveSelectedButton = nullptr;
    QPushButton *m_queueClearFinishedButton = nullptr;
    QLabel *m_queueStatusLabel = nullptr;
};
