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

    QLineEdit *m_localPathEdit = nullptr;
    QPushButton *m_localGoButton = nullptr;
    QPushButton *m_localUpButton = nullptr;
    QPushButton *m_localRefreshButton = nullptr;
    QPushButton *m_localUploadButton = nullptr;
    QLabel *m_localStatusLabel = nullptr;
    QFileSystemModel *m_localModel = nullptr;
    QTreeView *m_localTree = nullptr;

    QLineEdit *m_remotePathEdit = nullptr;
    QPushButton *m_remoteGoButton = nullptr;
    QPushButton *m_remoteUpButton = nullptr;
    QPushButton *m_remoteRefreshButton = nullptr;
    QPushButton *m_remoteDownloadButton = nullptr;
    QLabel *m_remoteStatusLabel = nullptr;
    QTableWidget *m_remoteTable = nullptr;
};
