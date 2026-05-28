#pragma once

#include "sftp/SftpProbe.h"

#include <QList>
#include <QString>
#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;

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
    void refreshDirectory();
    void openPathFromEditor();
    void goUp();
    void handleCellDoubleClicked(int row, int column);
    void populateTable(const QList<SftpRemoteEntry> &entries);
    void setBusy(bool busy);
    QString normalizedPath(const QString &path) const;
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
    QString m_currentPath = QStringLiteral(".");

    QLineEdit *m_pathEdit = nullptr;
    QPushButton *m_goButton = nullptr;
    QPushButton *m_upButton = nullptr;
    QPushButton *m_refreshButton = nullptr;
    QLabel *m_statusLabel = nullptr;
    QTableWidget *m_table = nullptr;
};
