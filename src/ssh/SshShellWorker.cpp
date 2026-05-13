#include "SshShellWorker.h"

#include <libssh/libssh.h>

#include <QByteArray>
#include <QDir>
#include <QFileDevice>
#include <QMutexLocker>
#include <QTemporaryFile>
#include <QThread>

SshShellWorker::SshShellWorker(
    const QString &host,
    int port,
    const QString &username,
    SshAuthMethod authMethod,
    const QString &secretValue,
    QObject *parent
)
    : QObject(parent)
    , m_host(host)
    , m_port(port)
    , m_username(username)
    , m_authMethod(authMethod)
    , m_secretValue(secretValue)
{
}

SshShellWorker::~SshShellWorker()
{
    requestStop();
}

void SshShellWorker::requestStop()
{
    m_stopRequested.store(true);
}

void SshShellWorker::stop()
{
    requestStop();
}

void SshShellWorker::sendInput(const QString &input)
{
    if (input.isEmpty()) {
        return;
    }

    QMutexLocker locker(&m_inputMutex);
    m_pendingInput.append(input);
}

void SshShellWorker::resizePty(int columns, int rows)
{
    if (columns <= 0 || rows <= 0) {
        return;
    }

    // This method is intentionally thread-safe. It may be called directly from
    // the GUI thread while the worker thread is running the libssh loop. The
    // latest size wins; the loop applies it when the channel is available.
    QMutexLocker locker(&m_resizeMutex);
    m_pendingColumns = columns;
    m_pendingRows = rows;
}

QString SshShellWorker::takePendingInput()
{
    QMutexLocker locker(&m_inputMutex);

    if (m_pendingInput.isEmpty()) {
        return QString();
    }

    const QString input = m_pendingInput.join(QString());
    m_pendingInput.clear();
    return input;
}

bool SshShellWorker::takePendingResize(int &columns, int &rows)
{
    QMutexLocker locker(&m_resizeMutex);

    if (m_pendingColumns <= 0 || m_pendingRows <= 0) {
        return false;
    }

    columns = m_pendingColumns;
    rows = m_pendingRows;
    m_pendingColumns = 0;
    m_pendingRows = 0;
    return true;
}

void SshShellWorker::start()
{
    emit stateChanged(QStringLiteral("Starting SSH shell worker..."));

    ssh_session session = ssh_new();

    if (session == nullptr) {
        emit errorOccurred(QStringLiteral("ssh_new() failed: could not allocate SSH session."));
        emit finished();
        return;
    }

    const QByteArray hostUtf8 = m_host.toUtf8();
    const QByteArray usernameUtf8 = m_username.toUtf8();

    int verbosity = SSH_LOG_NOLOG;
    long timeoutSeconds = 10;

    ssh_options_set(session, SSH_OPTIONS_HOST, hostUtf8.constData());
    ssh_options_set(session, SSH_OPTIONS_PORT, &m_port);
    ssh_options_set(session, SSH_OPTIONS_USER, usernameUtf8.constData());
    ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_options_set(session, SSH_OPTIONS_TIMEOUT, &timeoutSeconds);

    emit stateChanged(QStringLiteral("Connecting to SSH server..."));

    const int connectRc = ssh_connect(session);

    if (connectRc != SSH_OK) {
        emit errorOccurred(
            QStringLiteral("SSH connect failed: ")
            + QString::fromUtf8(ssh_get_error(session))
        );
        ssh_free(session);
        emit finished();
        return;
    }

    emit stateChanged(QStringLiteral("Authenticating..."));

    int authRc = SSH_AUTH_DENIED;
    QTemporaryFile tempKeyFile;

    if (m_authMethod == SshAuthMethod::Password) {
        const QByteArray passwordUtf8 = m_secretValue.toUtf8();

        authRc = ssh_userauth_password(
            session,
            usernameUtf8.constData(),
            passwordUtf8.constData()
        );
    } else {
        tempKeyFile.setFileTemplate(QDir::tempPath() + QStringLiteral("/dd-ssh-shell-key-XXXXXX"));
        tempKeyFile.setAutoRemove(true);

        if (!tempKeyFile.open()) {
            emit errorOccurred(QStringLiteral("Could not create temporary private key file: ") + tempKeyFile.errorString());
            ssh_disconnect(session);
            ssh_free(session);
            emit finished();
            return;
        }

        tempKeyFile.write(m_secretValue.toUtf8());
        tempKeyFile.flush();
        tempKeyFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
        const QString keyPath = tempKeyFile.fileName();
        tempKeyFile.close();

        ssh_key privateKey = nullptr;
        const QByteArray keyPathUtf8 = keyPath.toUtf8();

        const int importRc = ssh_pki_import_privkey_file(
            keyPathUtf8.constData(),
            nullptr,
            nullptr,
            nullptr,
            &privateKey
        );

        if (importRc != SSH_OK || privateKey == nullptr) {
            emit errorOccurred(
                QStringLiteral("Could not load private key from temporary file: ")
                + QString::fromUtf8(ssh_get_error(session))
            );
            ssh_disconnect(session);
            ssh_free(session);
            emit finished();
            return;
        }

        authRc = ssh_userauth_publickey(
            session,
            usernameUtf8.constData(),
            privateKey
        );

        ssh_key_free(privateKey);
    }

    if (authRc != SSH_AUTH_SUCCESS) {
        emit errorOccurred(
            QStringLiteral("SSH authentication failed. libssh auth return code: ")
            + QString::number(authRc)
            + QStringLiteral(". Error: ")
            + QString::fromUtf8(ssh_get_error(session))
        );
        ssh_disconnect(session);
        ssh_free(session);
        emit finished();
        return;
    }

    emit stateChanged(QStringLiteral("Authentication successful. Opening shell channel..."));

    ssh_channel channel = ssh_channel_new(session);

    if (channel == nullptr) {
        emit errorOccurred(QStringLiteral("ssh_channel_new() failed: could not allocate SSH channel."));
        ssh_disconnect(session);
        ssh_free(session);
        emit finished();
        return;
    }

    const int openRc = ssh_channel_open_session(channel);

    if (openRc != SSH_OK) {
        emit errorOccurred(
            QStringLiteral("Could not open SSH channel: ")
            + QString::fromUtf8(ssh_get_error(session))
        );
        ssh_channel_free(channel);
        ssh_disconnect(session);
        ssh_free(session);
        emit finished();
        return;
    }

    int requestedColumns = 0;
    int requestedRows = 0;
    const bool hasInitialSize = takePendingResize(requestedColumns, requestedRows);

    const int ptyRc = hasInitialSize
        ? ssh_channel_request_pty_size(channel, "xterm-256color", requestedColumns, requestedRows)
        : ssh_channel_request_pty(channel);

    if (ptyRc != SSH_OK) {
        emit errorOccurred(
            QStringLiteral("Could not request PTY: ")
            + QString::fromUtf8(ssh_get_error(session))
        );
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        ssh_disconnect(session);
        ssh_free(session);
        emit finished();
        return;
    }

    int currentColumns = hasInitialSize ? requestedColumns : 80;
    int currentRows = hasInitialSize ? requestedRows : 24;

    const int shellRc = ssh_channel_request_shell(channel);

    if (shellRc != SSH_OK) {
        emit errorOccurred(
            QStringLiteral("Could not request remote shell: ")
            + QString::fromUtf8(ssh_get_error(session))
        );
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        ssh_disconnect(session);
        ssh_free(session);
        emit finished();
        return;
    }

    ssh_set_blocking(session, 0);

    emit stateChanged(QStringLiteral("Connected. SSH shell channel is open."));
    emit outputReceived(QStringLiteral("\n[DD-SSH] SSH shell channel is open.\n\n"));

    char buffer[4096];

    while (!m_stopRequested.load()) {
        const QString pendingInput = takePendingInput();

        if (!pendingInput.isEmpty()) {
            const QByteArray inputUtf8 = pendingInput.toUtf8();
            const int writeRc = ssh_channel_write(channel, inputUtf8.constData(), inputUtf8.size());

            if (writeRc == SSH_ERROR) {
                emit errorOccurred(
                    QStringLiteral("Could not write to SSH channel: ")
                    + QString::fromUtf8(ssh_get_error(session))
                );
                break;
            }
        }

        int pendingColumns = 0;
        int pendingRows = 0;
        if (takePendingResize(pendingColumns, pendingRows)) {
            if (pendingColumns != currentColumns || pendingRows != currentRows) {
                const int resizeRc = ssh_channel_change_pty_size(channel, pendingColumns, pendingRows);

                if (resizeRc == SSH_OK) {
                    currentColumns = pendingColumns;
                    currentRows = pendingRows;
                } else {
                    emit errorOccurred(
                        QStringLiteral("Could not resize SSH PTY: ")
                        + QString::fromUtf8(ssh_get_error(session))
                    );
                }
            }
        }

        while (true) {
            const int bytesRead = ssh_channel_read_nonblocking(
                channel,
                buffer,
                sizeof(buffer),
                0
            );

            if (bytesRead > 0) {
                emit outputReceived(QString::fromUtf8(buffer, bytesRead));
                continue;
            }

            if (bytesRead == SSH_ERROR) {
                emit errorOccurred(
                    QStringLiteral("Could not read from SSH channel: ")
                    + QString::fromUtf8(ssh_get_error(session))
                );
            }

            break;
        }

        while (true) {
            const int bytesRead = ssh_channel_read_nonblocking(
                channel,
                buffer,
                sizeof(buffer),
                1
            );

            if (bytesRead > 0) {
                emit outputReceived(QString::fromUtf8(buffer, bytesRead));
                continue;
            }

            if (bytesRead == SSH_ERROR) {
                emit errorOccurred(
                    QStringLiteral("Could not read stderr from SSH channel: ")
                    + QString::fromUtf8(ssh_get_error(session))
                );
            }

            break;
        }

        if (ssh_channel_is_closed(channel) || ssh_channel_is_eof(channel)) {
            emit stateChanged(QStringLiteral("Remote shell channel closed."));
            break;
        }

        QThread::msleep(20);
    }

    emit stateChanged(QStringLiteral("Disconnecting SSH shell..."));

    if (channel != nullptr) {
        ssh_channel_send_eof(channel);
        ssh_channel_close(channel);
        ssh_channel_free(channel);
    }

    ssh_disconnect(session);
    ssh_free(session);

    emit stateChanged(QStringLiteral("Disconnected."));
    emit finished();
}
