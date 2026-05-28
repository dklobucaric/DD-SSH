#include "SshShellWorker.h"
#include "SshCompatibility.h"
#include "core/AppLogger.h"

#include <libssh/libssh.h>

#include <QByteArray>
#include <QDateTime>
#include <QDir>
#include <QFileDevice>
#include <QMutexLocker>
#include <QTemporaryFile>
#include <QThread>

namespace {

QString safeLogLabel(QString value)
{
    value = value.trimmed();

    if (value.isEmpty()) {
        return QStringLiteral("unnamed");
    }

    value.replace(QChar('"'), QChar('\''));
    value.replace(QChar('\n'), QChar(' '));
    value.replace(QChar('\r'), QChar(' '));
    return value;
}

QString formatByteCount(qint64 bytes)
{
    if (bytes < 0) {
        bytes = 0;
    }

    const double value = static_cast<double>(bytes);

    if (bytes < 1024) {
        return QString::number(bytes) + QStringLiteral(" B");
    }

    if (bytes < 1024LL * 1024LL) {
        return QString::number(value / 1024.0, 'f', 1) + QStringLiteral(" KB");
    }

    if (bytes < 1024LL * 1024LL * 1024LL) {
        return QString::number(value / (1024.0 * 1024.0), 'f', 1) + QStringLiteral(" MB");
    }

    return QString::number(value / (1024.0 * 1024.0 * 1024.0), 'f', 1) + QStringLiteral(" GB");
}

QString formatDuration(qint64 milliseconds)
{
    if (milliseconds < 0) {
        milliseconds = 0;
    }

    const qint64 totalSeconds = milliseconds / 1000;
    const qint64 hours = totalSeconds / 3600;
    const qint64 minutes = (totalSeconds % 3600) / 60;
    const qint64 seconds = totalSeconds % 60;

    return QStringLiteral("%1:%2:%3")
        .arg(hours, 2, 10, QLatin1Char('0'))
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'));
}

bool readConnectedServerHostKey(
    ssh_session session,
    QString *keyType,
    QString *fingerprint,
    QString *errorMessage
)
{
    if (keyType != nullptr) {
        *keyType = QString();
    }

    if (fingerprint != nullptr) {
        *fingerprint = QString();
    }

    if (errorMessage != nullptr) {
        *errorMessage = QString();
    }

    ssh_key serverPublicKey = nullptr;
    const int keyRc = ssh_get_server_publickey(session, &serverPublicKey);

    if (keyRc != SSH_OK || serverPublicKey == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Could not read SSH server host key: ")
                + QString::fromUtf8(ssh_get_error(session));
        }

        return false;
    }

    const char *rawKeyType = ssh_key_type_to_char(ssh_key_type(serverPublicKey));

    if (rawKeyType != nullptr) {
        if (keyType != nullptr) {
            *keyType = QString::fromUtf8(rawKeyType);
        }
    } else if (keyType != nullptr) {
        *keyType = QStringLiteral("unknown");
    }

    unsigned char *hash = nullptr;
    size_t hashLength = 0;

    const int hashRc = ssh_get_publickey_hash(
        serverPublicKey,
        SSH_PUBLICKEY_HASH_SHA256,
        &hash,
        &hashLength
    );

    if (hashRc != SSH_OK || hash == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Could not calculate SSH server host-key fingerprint: ")
                + QString::fromUtf8(ssh_get_error(session));
        }

        ssh_key_free(serverPublicKey);
        return false;
    }

    char *rawFingerprint = ssh_get_fingerprint_hash(
        SSH_PUBLICKEY_HASH_SHA256,
        hash,
        hashLength
    );

    ssh_clean_pubkey_hash(&hash);
    ssh_key_free(serverPublicKey);

    if (rawFingerprint == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Could not format SSH server host-key fingerprint.");
        }

        return false;
    }

    if (fingerprint != nullptr) {
        *fingerprint = QString::fromUtf8(rawFingerprint);
    }

    ssh_string_free_char(rawFingerprint);
    return true;
}

bool verifyConnectedServerHostKey(
    ssh_session session,
    const SshHostKeyExpectation &expectation,
    QString *errorMessage
)
{
    if (errorMessage != nullptr) {
        *errorMessage = QString();
    }

    if (!expectation.enabled) {
        return true;
    }

    QString currentKeyType;
    QString currentFingerprint;
    QString keyError;

    if (!readConnectedServerHostKey(session, &currentKeyType, &currentFingerprint, &keyError)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("SSH host key verification failed before authentication. Authentication was not attempted.\n\n")
                + keyError;
        }

        return false;
    }

    if (currentKeyType == expectation.keyType && currentFingerprint == expectation.fingerprint) {
        return true;
    }

    if (errorMessage != nullptr) {
        *errorMessage =
            QStringLiteral("SSH host key verification failed before authentication. Authentication was not attempted.\n\n")
            + QStringLiteral("The real shell connection reported a different SSH host key than the key approved during the preflight check.\n\n")
            + QStringLiteral("Expected host: ") + expectation.host + QStringLiteral(":") + QString::number(expectation.port) + QStringLiteral("\n")
            + QStringLiteral("Expected key type: ") + expectation.keyType + QStringLiteral("\n")
            + QStringLiteral("Expected fingerprint: ") + expectation.fingerprint + QStringLiteral("\n")
            + QStringLiteral("Current key type: ") + currentKeyType + QStringLiteral("\n")
            + QStringLiteral("Current fingerprint: ") + currentFingerprint;
    }

    return false;
}

}

SshShellWorker::SshShellWorker(
    const QString &host,
    int port,
    const QString &username,
    SshAuthMethod authMethod,
    const QString &secretValue,
    const SshHostKeyExpectation &hostKeyExpectation,
    const QString &sessionLabel,
    QObject *parent
)
    : QObject(parent)
    , m_host(host)
    , m_port(port)
    , m_username(username)
    , m_authMethod(authMethod)
    , m_secretValue(secretValue)
    , m_hostKeyExpectation(hostKeyExpectation)
    , m_sessionLabel(sessionLabel)
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
    AppLogger::info(QStringLiteral("SSH shell worker started: host=") + m_host
        + QStringLiteral(", port=") + QString::number(m_port)
        + QStringLiteral(", user=") + m_username
        + QStringLiteral(", method=") + (m_authMethod == SshAuthMethod::Password ? QStringLiteral("password") : QStringLiteral("private-key")));
    emit stateChanged(QStringLiteral("Starting SSH shell worker..."));

    ssh_session session = ssh_new();

    if (session == nullptr) {
        AppLogger::error(QStringLiteral("SSH shell worker failed before connect: ssh_new failed"));
        emit errorOccurred(QStringLiteral("ssh_new() failed: could not allocate SSH session."));
        emit finished();
        return;
    }

    const QByteArray hostUtf8 = m_host.toUtf8();
    const QByteArray usernameUtf8 = m_username.toUtf8();

    int verbosity = SshCompatibility::defaultLogVerbosity();
    long timeoutSeconds = 10;

    ssh_options_set(session, SSH_OPTIONS_HOST, hostUtf8.constData());
    ssh_options_set(session, SSH_OPTIONS_PORT, &m_port);
    ssh_options_set(session, SSH_OPTIONS_USER, usernameUtf8.constData());
    ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_options_set(session, SSH_OPTIONS_TIMEOUT, &timeoutSeconds);
    SshCompatibility::applySessionCompatibility(session);

    emit stateChanged(QStringLiteral("Connecting to SSH server..."));
    AppLogger::info(QStringLiteral("SSH shell connect started: host=") + m_host
        + QStringLiteral(", port=") + QString::number(m_port));

    const int connectRc = ssh_connect(session);

    if (connectRc != SSH_OK) {
        const QString connectError = QString::fromUtf8(ssh_get_error(session));
        AppLogger::error(QStringLiteral("SSH shell connect failed: host=") + m_host
            + QStringLiteral(", port=") + QString::number(m_port)
            + QStringLiteral(", error=") + connectError);
        emit errorOccurred(
            QStringLiteral("SSH connect failed: ")
            + connectError
        );
        ssh_free(session);
        emit finished();
        return;
    }

    AppLogger::info(QStringLiteral("SSH shell connect OK: host=") + m_host
        + QStringLiteral(", port=") + QString::number(m_port));

    if (m_hostKeyExpectation.enabled) {
        emit stateChanged(QStringLiteral("Verifying SSH host key before authentication..."));

        QString verificationError;
        if (!verifyConnectedServerHostKey(session, m_hostKeyExpectation, &verificationError)) {
            AppLogger::error(QStringLiteral("SSH shell host-key verification failed before auth: host=") + m_host
                + QStringLiteral(", port=") + QString::number(m_port)
                + QStringLiteral(", error=") + verificationError);
            emit errorOccurred(verificationError);
            ssh_disconnect(session);
            ssh_free(session);
            emit finished();
            return;
        }

        AppLogger::info(QStringLiteral("SSH shell host-key verification OK: host=") + m_host
            + QStringLiteral(", port=") + QString::number(m_port));
    }

    emit stateChanged(QStringLiteral("Authenticating..."));
    AppLogger::info(QStringLiteral("SSH shell authentication started: host=") + m_host
        + QStringLiteral(", port=") + QString::number(m_port)
        + QStringLiteral(", method=") + (m_authMethod == SshAuthMethod::Password ? QStringLiteral("password") : QStringLiteral("private-key")));

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
            AppLogger::error(QStringLiteral("SSH shell private-key temporary file create failed: ") + tempKeyFile.errorString());
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
            const QString keyError = QString::fromUtf8(ssh_get_error(session));
            AppLogger::error(QStringLiteral("SSH shell private-key load failed: host=") + m_host
                + QStringLiteral(", port=") + QString::number(m_port)
                + QStringLiteral(", error=") + keyError);
            emit errorOccurred(
                QStringLiteral("Could not load private key from temporary file: ")
                + keyError
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
        const QString authError = QString::fromUtf8(ssh_get_error(session));
        AppLogger::warn(QStringLiteral("SSH shell authentication failed: host=") + m_host
            + QStringLiteral(", port=") + QString::number(m_port)
            + QStringLiteral(", method=") + (m_authMethod == SshAuthMethod::Password ? QStringLiteral("password") : QStringLiteral("private-key"))
            + QStringLiteral(", rc=") + QString::number(authRc)
            + QStringLiteral(", error=") + authError);
        emit errorOccurred(
            QStringLiteral("SSH authentication failed. libssh auth return code: ")
            + QString::number(authRc)
            + QStringLiteral(". Error: ")
            + authError
        );
        ssh_disconnect(session);
        ssh_free(session);
        emit finished();
        return;
    }

    AppLogger::info(QStringLiteral("SSH shell authentication successful: host=") + m_host
        + QStringLiteral(", port=") + QString::number(m_port));
    emit stateChanged(QStringLiteral("Authentication successful. Opening shell channel..."));

    ssh_channel channel = ssh_channel_new(session);

    if (channel == nullptr) {
        AppLogger::error(QStringLiteral("SSH shell channel allocation failed"));
        emit errorOccurred(QStringLiteral("ssh_channel_new() failed: could not allocate SSH channel."));
        ssh_disconnect(session);
        ssh_free(session);
        emit finished();
        return;
    }

    const int openRc = ssh_channel_open_session(channel);

    if (openRc != SSH_OK) {
        const QString channelError = QString::fromUtf8(ssh_get_error(session));
        AppLogger::error(QStringLiteral("SSH shell channel open failed: ") + channelError);
        emit errorOccurred(
            QStringLiteral("Could not open SSH channel: ")
            + channelError
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

    AppLogger::info(QStringLiteral("SSH shell channel open: host=") + m_host
        + QStringLiteral(", port=") + QString::number(m_port));
    emit stateChanged(QStringLiteral("Connected. SSH shell channel is open."));
    emit outputReceived(QStringLiteral("\n[DD-SSH] SSH shell channel is open.\n\n"));

    const QString trafficSessionLabel = safeLogLabel(!m_sessionLabel.trimmed().isEmpty() ? m_sessionLabel : (m_username + QStringLiteral("@") + m_host));
    qint64 receivedBytesTotal = 0;
    qint64 sentBytesTotal = 0;
    const QDateTime trafficStartedAt = QDateTime::currentDateTime();

    AppLogger::info(QStringLiteral("Traffic monitor started: session=\"") + trafficSessionLabel + QStringLiteral("\""));
    emit trafficUpdated(receivedBytesTotal, sentBytesTotal);

    char buffer[4096];
    bool connectionLost = false;
    bool remoteClosed = false;

    while (!m_stopRequested.load()) {
        if (ssh_is_connected(session) == 0) {
            connectionLost = true;
            AppLogger::warn(QStringLiteral("SSH shell transport disconnected: host=") + m_host
                + QStringLiteral(", port=") + QString::number(m_port));
            emit errorOccurred(QStringLiteral("SSH transport disconnected. The remote host may have closed the connection or rebooted."));
            break;
        }

        const QString pendingInput = takePendingInput();

        if (!pendingInput.isEmpty()) {
            const QByteArray inputUtf8 = pendingInput.toUtf8();
            const int writeRc = ssh_channel_write(channel, inputUtf8.constData(), inputUtf8.size());

            if (writeRc > 0) {
                sentBytesTotal += writeRc;
                emit trafficUpdated(receivedBytesTotal, sentBytesTotal);
            }

            if (writeRc == SSH_ERROR) {
                connectionLost = true;
                const QString writeError = QString::fromUtf8(ssh_get_error(session));
                AppLogger::error(QStringLiteral("SSH shell write failed: ") + writeError);
                emit errorOccurred(
                    QStringLiteral("Could not write to SSH channel. Connection may be closed: ")
                    + writeError
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

        bool readError = false;

        while (true) {
            const int bytesRead = ssh_channel_read_nonblocking(
                channel,
                buffer,
                sizeof(buffer),
                0
            );

            if (bytesRead > 0) {
                receivedBytesTotal += bytesRead;
                emit trafficUpdated(receivedBytesTotal, sentBytesTotal);
                emit outputReceived(QString::fromUtf8(buffer, bytesRead));
                continue;
            }

            if (bytesRead == SSH_ERROR) {
                readError = true;
                emit errorOccurred(
                    QStringLiteral("Could not read from SSH channel. Connection may be closed: ")
                    + QString::fromUtf8(ssh_get_error(session))
                );
            }

            break;
        }

        if (readError) {
            connectionLost = true;
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
                receivedBytesTotal += bytesRead;
                emit trafficUpdated(receivedBytesTotal, sentBytesTotal);
                emit outputReceived(QString::fromUtf8(buffer, bytesRead));
                continue;
            }

            if (bytesRead == SSH_ERROR) {
                readError = true;
                emit errorOccurred(
                    QStringLiteral("Could not read stderr from SSH channel. Connection may be closed: ")
                    + QString::fromUtf8(ssh_get_error(session))
                );
            }

            break;
        }

        if (readError) {
            connectionLost = true;
            break;
        }

        if (ssh_channel_is_closed(channel) || ssh_channel_is_eof(channel)) {
            remoteClosed = true;
            AppLogger::info(QStringLiteral("Remote shell channel closed: host=") + m_host
                + QStringLiteral(", port=") + QString::number(m_port));
            emit stateChanged(QStringLiteral("Remote shell channel closed."));
            break;
        }

        QThread::msleep(20);
    }

    if (m_stopRequested.load()) {
        emit stateChanged(QStringLiteral("Disconnecting SSH shell..."));
    } else if (connectionLost) {
        emit stateChanged(QStringLiteral("SSH connection lost. Cleaning up shell session..."));
    } else if (remoteClosed) {
        emit stateChanged(QStringLiteral("Remote shell closed. Cleaning up shell session..."));
    } else {
        emit stateChanged(QStringLiteral("Cleaning up SSH shell session..."));
    }

    if (channel != nullptr) {
        ssh_channel_send_eof(channel);
        ssh_channel_close(channel);
        ssh_channel_free(channel);
    }

    ssh_disconnect(session);
    ssh_free(session);

    const qint64 trafficDurationMs = trafficStartedAt.msecsTo(QDateTime::currentDateTime());
    AppLogger::info(QStringLiteral("Session traffic summary: session=\"") + trafficSessionLabel
        + QStringLiteral("\", duration=") + formatDuration(trafficDurationMs)
        + QStringLiteral(", received=") + formatByteCount(receivedBytesTotal)
        + QStringLiteral(", sent=") + formatByteCount(sentBytesTotal));
    AppLogger::info(QStringLiteral("Traffic monitor stopped: session=\"") + trafficSessionLabel
        + QStringLiteral("\", received=") + formatByteCount(receivedBytesTotal)
        + QStringLiteral(", sent=") + formatByteCount(sentBytesTotal));

    AppLogger::info(QStringLiteral("SSH shell disconnected: host=") + m_host
        + QStringLiteral(", port=") + QString::number(m_port));
    emit stateChanged(QStringLiteral("Disconnected."));
    emit finished();
}
