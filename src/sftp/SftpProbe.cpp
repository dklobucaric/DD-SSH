#include "SftpProbe.h"
#include "ssh/SshCompatibility.h"
#include "core/AppLogger.h"

#include <libssh/libssh.h>
#include <libssh/sftp.h>

#include <QByteArray>
#include <QDateTime>
#include <QDir>
#include <QFileDevice>
#include <QTemporaryFile>

#include <cstdint>

namespace {

QString fileTypeLabel(int type)
{
    switch (type) {
    case SSH_FILEXFER_TYPE_REGULAR:
        return QStringLiteral("file");
    case SSH_FILEXFER_TYPE_DIRECTORY:
        return QStringLiteral("directory");
    case SSH_FILEXFER_TYPE_SYMLINK:
        return QStringLiteral("symlink");
    case SSH_FILEXFER_TYPE_SPECIAL:
        return QStringLiteral("special");
    case SSH_FILEXFER_TYPE_UNKNOWN:
    default:
        return QStringLiteral("unknown");
    }
}

QString permissionString(uint32_t permissions)
{
    if (permissions == 0) {
        return QStringLiteral("(unknown)");
    }

    return QStringLiteral("0") + QStringLiteral("%1")
        .arg(static_cast<unsigned int>(permissions & 07777U), 4, 8, QLatin1Char('0'));
}

QString timestampString(uint32_t timestamp)
{
    if (timestamp == 0) {
        return QStringLiteral("(unknown)");
    }

    return QDateTime::fromSecsSinceEpoch(static_cast<qint64>(timestamp)).toString(Qt::ISODate);
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
    SftpProbeResult *result
)
{
    if (!expectation.enabled) {
        return true;
    }

    if (result != nullptr) {
        result->hostKeyVerificationAttempted = true;
    }

    QString currentKeyType;
    QString currentFingerprint;
    QString keyError;

    if (!readConnectedServerHostKey(session, &currentKeyType, &currentFingerprint, &keyError)) {
        if (result != nullptr) {
            result->success = false;
            result->message = QStringLiteral("SSH host key verification failed before SFTP authentication.");
            result->error = keyError;
            result->sshErrorCode = ssh_get_error_code(session);
            result->hostKeyVerified = false;
            result->hostKeyType = currentKeyType;
            result->hostKeyFingerprint = currentFingerprint;
        }

        return false;
    }

    if (result != nullptr) {
        result->hostKeyType = currentKeyType;
        result->hostKeyFingerprint = currentFingerprint;
    }

    const bool matches =
        currentKeyType == expectation.keyType
        && currentFingerprint == expectation.fingerprint;

    if (!matches) {
        if (result != nullptr) {
            result->success = false;
            result->message = QStringLiteral("SSH host key verification failed before SFTP authentication.");
            result->error =
                QStringLiteral("The SFTP connection reported a different SSH host key than the key approved during the preflight check. Authentication was not attempted.\n")
                + QStringLiteral("Expected host: ") + expectation.host + QStringLiteral(":") + QString::number(expectation.port) + QStringLiteral("\n")
                + QStringLiteral("Expected key type: ") + expectation.keyType + QStringLiteral("\n")
                + QStringLiteral("Expected fingerprint: ") + expectation.fingerprint + QStringLiteral("\n")
                + QStringLiteral("Current key type: ") + currentKeyType + QStringLiteral("\n")
                + QStringLiteral("Current fingerprint: ") + currentFingerprint;
            result->sshErrorCode = ssh_get_error_code(session);
            result->hostKeyVerified = false;
        }

        return false;
    }

    if (result != nullptr) {
        result->hostKeyVerified = true;
    }

    return true;
}

bool authenticateSession(
    ssh_session session,
    const QString &username,
    SshAuthMethod authMethod,
    const QString &secretValue,
    SftpProbeResult *result
)
{
    const QByteArray usernameUtf8 = username.toUtf8();
    int authRc = SSH_AUTH_DENIED;
    QTemporaryFile tempKeyFile;

    if (authMethod == SshAuthMethod::Password) {
        const QByteArray passwordUtf8 = secretValue.toUtf8();
        authRc = ssh_userauth_password(
            session,
            usernameUtf8.constData(),
            passwordUtf8.constData()
        );
    } else {
        tempKeyFile.setFileTemplate(QDir::tempPath() + QStringLiteral("/dd-ssh-sftp-key-XXXXXX"));
        tempKeyFile.setAutoRemove(true);

        if (!tempKeyFile.open()) {
            if (result != nullptr) {
                result->success = false;
                result->message = QStringLiteral("Could not create temporary private key file for SFTP directory listing.");
                result->error = tempKeyFile.errorString();
                result->sshErrorCode = ssh_get_error_code(session);
            }

            return false;
        }

        tempKeyFile.write(secretValue.toUtf8());
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
            if (result != nullptr) {
                result->success = false;
                result->message = QStringLiteral("Could not load private key for SFTP directory listing.");
                result->authReturnCode = importRc;
                result->sshErrorCode = ssh_get_error_code(session);
                result->error = QString::fromUtf8(ssh_get_error(session));
            }

            return false;
        }

        authRc = ssh_userauth_publickey(
            session,
            usernameUtf8.constData(),
            privateKey
        );

        ssh_key_free(privateKey);
    }

    if (result != nullptr) {
        result->authReturnCode = authRc;
    }

    if (authRc != SSH_AUTH_SUCCESS) {
        if (result != nullptr) {
            result->success = false;
            result->sshErrorCode = ssh_get_error_code(session);
            result->error = QString::fromUtf8(ssh_get_error(session));

            if (authRc == SSH_AUTH_DENIED) {
                result->message = QStringLiteral("SFTP directory listing authentication denied by server.");
            } else if (authRc == SSH_AUTH_PARTIAL) {
                result->message = QStringLiteral("SFTP directory listing authentication partially successful; additional method required.");
            } else if (authRc == SSH_AUTH_AGAIN) {
                result->message = QStringLiteral("SFTP directory listing authentication needs to be retried.");
            } else {
                result->message = QStringLiteral("SFTP directory listing authentication failed.");
            }
        }

        return false;
    }

    return true;
}

} // namespace

SftpProbeResult SftpProbe::listRemoteDirectory(
    const QString &host,
    int port,
    const QString &username,
    SshAuthMethod authMethod,
    const QString &secretValue,
    const SshHostKeyExpectation &hostKeyExpectation,
    const QString &remotePath
)
{
    SftpProbeResult result;
    result.remotePath = remotePath.trimmed().isEmpty() ? QStringLiteral(".") : remotePath.trimmed();

    AppLogger::info(QStringLiteral("SFTP directory listing started: host=") + host
        + QStringLiteral(", port=") + QString::number(port)
        + QStringLiteral(", user=") + username
        + QStringLiteral(", method=") + (authMethod == SshAuthMethod::Password ? QStringLiteral("password") : QStringLiteral("private-key")));

    ssh_session session = ssh_new();

    if (session == nullptr) {
        result.success = false;
        result.message = QStringLiteral("SFTP directory listing failed before connect.");
        result.error = QStringLiteral("ssh_new() failed: could not allocate SSH session.");
        AppLogger::error(QStringLiteral("SFTP directory listing failed before connect: ssh_new failed"));
        return result;
    }

    const QByteArray hostUtf8 = host.toUtf8();
    const QByteArray usernameUtf8 = username.toUtf8();

    int verbosity = SshCompatibility::defaultLogVerbosity();
    long timeoutSeconds = 10;

    ssh_options_set(session, SSH_OPTIONS_HOST, hostUtf8.constData());
    ssh_options_set(session, SSH_OPTIONS_PORT, &port);
    ssh_options_set(session, SSH_OPTIONS_USER, usernameUtf8.constData());
    ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_options_set(session, SSH_OPTIONS_TIMEOUT, &timeoutSeconds);
    SshCompatibility::applySessionCompatibility(session);

    const int connectRc = ssh_connect(session);

    if (connectRc != SSH_OK) {
        result.success = false;
        result.message = QStringLiteral("SSH connect failed during SFTP directory listing.");
        result.sshErrorCode = ssh_get_error_code(session);
        result.error = QString::fromUtf8(ssh_get_error(session));
        AppLogger::error(QStringLiteral("SFTP directory listing connect failed: host=") + host
            + QStringLiteral(", port=") + QString::number(port)
            + QStringLiteral(", error=") + result.error);
        ssh_free(session);
        return result;
    }

    if (!verifyConnectedServerHostKey(session, hostKeyExpectation, &result)) {
        AppLogger::error(QStringLiteral("SFTP directory listing host-key verification failed before auth: host=") + host
            + QStringLiteral(", port=") + QString::number(port)
            + QStringLiteral(", error=") + result.error);
        ssh_disconnect(session);
        ssh_free(session);
        return result;
    }

    if (hostKeyExpectation.enabled) {
        AppLogger::info(QStringLiteral("SFTP directory listing host-key verification OK: host=") + host
            + QStringLiteral(", port=") + QString::number(port));
    }

    if (!authenticateSession(session, username, authMethod, secretValue, &result)) {
        AppLogger::warn(QStringLiteral("SFTP directory listing authentication failed: host=") + host
            + QStringLiteral(", port=") + QString::number(port)
            + QStringLiteral(", message=") + result.message
            + QStringLiteral(", error=") + result.error);
        ssh_disconnect(session);
        ssh_free(session);
        return result;
    }

    AppLogger::info(QStringLiteral("SFTP directory listing authentication successful: host=") + host
        + QStringLiteral(", port=") + QString::number(port));

    sftp_session sftp = sftp_new(session);

    if (sftp == nullptr) {
        result.success = false;
        result.message = QStringLiteral("Could not allocate SFTP session.");
        result.sshErrorCode = ssh_get_error_code(session);
        result.error = QString::fromUtf8(ssh_get_error(session));
        AppLogger::error(QStringLiteral("SFTP directory listing allocation failed: ") + result.error);
        ssh_disconnect(session);
        ssh_free(session);
        return result;
    }

    const int initRc = sftp_init(sftp);

    if (initRc != SSH_OK) {
        result.success = false;
        result.message = QStringLiteral("Could not initialize SFTP subsystem.");
        result.sftpErrorCode = sftp_get_error(sftp);
        result.error = QString::fromUtf8(ssh_get_error(session));
        AppLogger::error(QStringLiteral("SFTP directory listing subsystem init failed: ") + result.error
            + QStringLiteral(", sftpError=") + QString::number(result.sftpErrorCode));
        sftp_free(sftp);
        ssh_disconnect(session);
        ssh_free(session);
        return result;
    }

    const QByteArray pathUtf8 = result.remotePath.toUtf8();
    sftp_dir directory = sftp_opendir(sftp, pathUtf8.constData());

    if (directory == nullptr) {
        result.success = false;
        result.message = QStringLiteral("Could not open remote directory for SFTP directory listing.");
        result.sftpErrorCode = sftp_get_error(sftp);
        result.error = QString::fromUtf8(ssh_get_error(session));
        AppLogger::error(QStringLiteral("SFTP directory listing opendir failed: sftpError=") + QString::number(result.sftpErrorCode)
            + QStringLiteral(", error=") + result.error);
        sftp_free(sftp);
        ssh_disconnect(session);
        ssh_free(session);
        return result;
    }

    while (true) {
        sftp_attributes attributes = sftp_readdir(sftp, directory);

        if (attributes == nullptr) {
            break;
        }

        SftpRemoteEntry entry;
        entry.name = attributes->name != nullptr ? QString::fromUtf8(attributes->name) : QStringLiteral("(unnamed)");
        entry.type = fileTypeLabel(attributes->type);
        entry.sizeBytes = static_cast<quint64>(attributes->size);
        entry.modifiedTime = timestampString(attributes->mtime);
        entry.permissions = permissionString(attributes->permissions);
        result.entries.append(entry);

        sftp_attributes_free(attributes);
    }

    const bool reachedEnd = sftp_dir_eof(directory) != 0;

    if (!reachedEnd) {
        result.success = false;
        result.message = QStringLiteral("Remote directory listing stopped before EOF.");
        result.sftpErrorCode = sftp_get_error(sftp);
        result.error = QString::fromUtf8(ssh_get_error(session));
        AppLogger::warn(QStringLiteral("SFTP directory listing readdir stopped before EOF: sftpError=") + QString::number(result.sftpErrorCode)
            + QStringLiteral(", error=") + result.error);
        sftp_closedir(directory);
        sftp_free(sftp);
        ssh_disconnect(session);
        ssh_free(session);
        return result;
    }

    sftp_closedir(directory);
    sftp_free(sftp);
    ssh_disconnect(session);
    ssh_free(session);

    result.success = true;
    result.message = QStringLiteral("SFTP subsystem initialized and remote directory listed successfully.");

    AppLogger::info(QStringLiteral("SFTP directory listing successful: host=") + host
        + QStringLiteral(", port=") + QString::number(port)
        + QStringLiteral(", entries=") + QString::number(result.entries.size()));

    return result;
}
