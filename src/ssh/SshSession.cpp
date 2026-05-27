#include "SshSession.h"
#include "SshCompatibility.h"

#include <libssh/libssh.h>

#include <QByteArray>
#include <QDir>

namespace {

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
    SshAuthResult *result
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
            result->message = QStringLiteral("SSH host key verification failed before authentication.");
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
            result->message = QStringLiteral("SSH host key verification failed before authentication.");
            result->error =
                QStringLiteral("The real authentication connection reported a different SSH host key than the key approved during the preflight check. Authentication was not attempted.\n")
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

}

QString SshSession::libsshVersion()
{
    const char *version = ssh_version(0);

    if (version == nullptr) {
        return QStringLiteral("unknown");
    }

    return QString::fromUtf8(version);
}

SshHandshakeResult SshSession::testHandshake(
    const QString &host,
    int port,
    const QString &username
)
{
    SshHandshakeResult result;

    ssh_session session = ssh_new();

    if (session == nullptr) {
        result.success = false;
        result.error = QStringLiteral("ssh_new() failed: could not allocate SSH session.");
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

    const int rc = ssh_connect(session);

    if (rc != SSH_OK) {
        result.success = false;
        result.sshErrorCode = ssh_get_error_code(session);
        result.error = QString::fromUtf8(ssh_get_error(session));
        result.message = QStringLiteral("SSH handshake failed.");

        ssh_free(session);
        return result;
    }

    const char *banner = ssh_get_serverbanner(session);

    result.success = true;
    result.message = QStringLiteral("SSH handshake successful. Authentication was not attempted yet.");

    if (banner != nullptr) {
        result.serverBanner = QString::fromUtf8(banner);
    } else {
        result.serverBanner = QStringLiteral("(server banner unavailable)");
    }

    QString keyError;
    if (!readConnectedServerHostKey(session, &result.hostKeyType, &result.hostKeyFingerprint, &keyError)) {
        result.hostKeyType = QStringLiteral("(host key unavailable)");
        result.hostKeyFingerprint = QStringLiteral("(fingerprint unavailable)");
        result.error = keyError;
    }

    ssh_disconnect(session);
    ssh_free(session);

    return result;
}

SshAuthResult SshSession::testAuthentication(
    const QString &host,
    int port,
    const QString &username,
    SshAuthMethod authMethod,
    const QString &password,
    const QString &privateKeyPath,
    const SshHostKeyExpectation &hostKeyExpectation
)
{
    SshAuthResult result;

    ssh_session session = ssh_new();

    if (session == nullptr) {
        result.success = false;
        result.error = QStringLiteral("ssh_new() failed: could not allocate SSH session.");
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
        result.sshErrorCode = ssh_get_error_code(session);
        result.error = QString::fromUtf8(ssh_get_error(session));
        result.message = QStringLiteral("SSH connect failed during authentication test.");

        ssh_free(session);
        return result;
    }

    if (!verifyConnectedServerHostKey(session, hostKeyExpectation, &result)) {
        ssh_disconnect(session);
        ssh_free(session);
        return result;
    }

    int authRc = SSH_AUTH_DENIED;

    if (authMethod == SshAuthMethod::Password) {
        const QByteArray passwordUtf8 = password.toUtf8();

        authRc = ssh_userauth_password(
            session,
            usernameUtf8.constData(),
            passwordUtf8.constData()
        );
    } else {
        QString expandedKeyPath = privateKeyPath.trimmed();

        if (expandedKeyPath.startsWith(QStringLiteral("~/"))) {
            expandedKeyPath = QDir::homePath() + expandedKeyPath.mid(1);
        }

        const QByteArray keyPathUtf8 = expandedKeyPath.toUtf8();

        ssh_key privateKey = nullptr;

        const int importRc = ssh_pki_import_privkey_file(
            keyPathUtf8.constData(),
            nullptr,
            nullptr,
            nullptr,
            &privateKey
        );

        if (importRc != SSH_OK || privateKey == nullptr) {
            result.success = false;
            result.authReturnCode = importRc;
            result.sshErrorCode = ssh_get_error_code(session);
            result.error = QString::fromUtf8(ssh_get_error(session));
            result.message = QStringLiteral("Could not load private key file.");

            ssh_disconnect(session);
            ssh_free(session);
            return result;
        }

        authRc = ssh_userauth_publickey(
            session,
            usernameUtf8.constData(),
            privateKey
        );

        ssh_key_free(privateKey);
    }

    result.authReturnCode = authRc;

    if (authRc == SSH_AUTH_SUCCESS) {
        result.success = true;

        if (authMethod == SshAuthMethod::Password) {
            result.message = QStringLiteral("Password authentication successful.");
        } else {
            result.message = QStringLiteral("Private-key authentication successful.");
        }
    } else {
        result.success = false;
        result.sshErrorCode = ssh_get_error_code(session);
        result.error = QString::fromUtf8(ssh_get_error(session));

        if (authRc == SSH_AUTH_DENIED) {
            result.message = QStringLiteral("Authentication denied by server.");
        } else if (authRc == SSH_AUTH_PARTIAL) {
            result.message = QStringLiteral("Authentication partially successful; additional method required.");
        } else if (authRc == SSH_AUTH_AGAIN) {
            result.message = QStringLiteral("Authentication needs to be retried.");
        } else {
            result.message = QStringLiteral("Authentication failed.");
        }
    }

    ssh_disconnect(session);
    ssh_free(session);

    return result;
}
