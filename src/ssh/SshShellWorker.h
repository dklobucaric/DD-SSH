#pragma once

#include "SshSession.h"

#include <QObject>
#include <QMutex>
#include <QString>
#include <QStringList>
#include <atomic>

class SshShellWorker : public QObject
{
    Q_OBJECT

public:
    SshShellWorker(
        const QString &host,
        int port,
        const QString &username,
        SshAuthMethod authMethod,
        const QString &secretValue,
        QObject *parent = nullptr
    );

    ~SshShellWorker() override;

public slots:
    void start();
    void stop();
    void sendInput(const QString &input);
    void resizePty(int columns, int rows);

signals:
    void outputReceived(const QString &output);
    void stateChanged(const QString &state);
    void errorOccurred(const QString &error);
    void finished();

private:
    QString takePendingInput();
    bool takePendingResize(int &columns, int &rows);
    void requestStop();

    QString m_host;
    int m_port = 22;
    QString m_username;
    SshAuthMethod m_authMethod = SshAuthMethod::Password;
    QString m_secretValue;

    QMutex m_inputMutex;
    QStringList m_pendingInput;

    QMutex m_resizeMutex;
    int m_pendingColumns = 0;
    int m_pendingRows = 0;
    std::atomic_bool m_stopRequested { false };
};
