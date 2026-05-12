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

    ssh_disconnect(session);
    ssh_free(session);

    return result;
}
