#include "SshSession.h"

#include <libssh/libssh.h>

#include <QByteArray>

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

    int verbosity = SSH_LOG_NOLOG;
    long timeoutSeconds = 10;

    ssh_options_set(session, SSH_OPTIONS_HOST, hostUtf8.constData());
    ssh_options_set(session, SSH_OPTIONS_PORT, &port);
    ssh_options_set(session, SSH_OPTIONS_USER, usernameUtf8.constData());
    ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_options_set(session, SSH_OPTIONS_TIMEOUT, &timeoutSeconds);

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

    ssh_key serverPublicKey = nullptr;
    const int keyRc = ssh_get_server_publickey(session, &serverPublicKey);

    if (keyRc == SSH_OK && serverPublicKey != nullptr) {
        const char *keyType = ssh_key_type_to_char(ssh_key_type(serverPublicKey));

        if (keyType != nullptr) {
            result.hostKeyType = QString::fromUtf8(keyType);
        } else {
            result.hostKeyType = QStringLiteral("unknown");
        }

        unsigned char *hash = nullptr;
        size_t hashLength = 0;

        const int hashRc = ssh_get_publickey_hash(
            serverPublicKey,
            SSH_PUBLICKEY_HASH_SHA256,
            &hash,
            &hashLength
        );

        if (hashRc == SSH_OK && hash != nullptr) {
            char *fingerprint = ssh_get_fingerprint_hash(
                SSH_PUBLICKEY_HASH_SHA256,
                hash,
                hashLength
            );

            if (fingerprint != nullptr) {
                result.hostKeyFingerprint = QString::fromUtf8(fingerprint);
                ssh_string_free_char(fingerprint);
            } else {
                result.hostKeyFingerprint = QStringLiteral("(fingerprint unavailable)");
            }

            ssh_clean_pubkey_hash(&hash);
        } else {
            result.hostKeyFingerprint = QStringLiteral("(public key hash unavailable)");
        }

        ssh_key_free(serverPublicKey);
    } else {
        result.hostKeyType = QStringLiteral("(host key unavailable)");
        result.hostKeyFingerprint = QStringLiteral("(fingerprint unavailable)");
    }

    ssh_disconnect(session);
    ssh_free(session);

    return result;
}
