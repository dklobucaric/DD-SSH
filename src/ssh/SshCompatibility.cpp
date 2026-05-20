#include "SshCompatibility.h"

#include <QByteArray>
#include <QStringList>
#include <QtGlobal>

namespace
{
#ifdef Q_OS_WIN
constexpr const char *kWindowsSafeKexAlgorithms =
    "curve25519-sha256,"
    "curve25519-sha256@libssh.org,"
    "ecdh-sha2-nistp256,"
    "ecdh-sha2-nistp384,"
    "ecdh-sha2-nistp521,"
    "diffie-hellman-group14-sha256";
#endif

bool environmentFlagEnabled(const char *name)
{
    const QByteArray value = qgetenv(name).trimmed().toLower();

    return value == "1"
        || value == "true"
        || value == "yes"
        || value == "on";
}

bool environmentFlagDisabled(const char *name)
{
    const QByteArray value = qgetenv(name).trimmed().toLower();

    return value == "1"
        || value == "true"
        || value == "yes"
        || value == "on";
}
}

namespace SshCompatibility
{
int defaultLogVerbosity()
{
    if (environmentFlagEnabled("DD_SSH_LIBSSH_DEBUG")) {
        return SSH_LOG_PROTOCOL;
    }

    return SSH_LOG_NOLOG;
}

void applySessionCompatibility(ssh_session session)
{
    if (session == nullptr) {
        return;
    }

#ifdef Q_OS_WIN
    if (!environmentFlagDisabled("DD_SSH_DISABLE_WINDOWS_KEX_COMPAT")) {
        const int rc = ssh_options_set(
            session,
            SSH_OPTIONS_KEY_EXCHANGE,
            kWindowsSafeKexAlgorithms
        );
        (void)rc;
    }
#endif
}

QString activeCompatibilityNotes()
{
    QStringList notes;

#ifdef Q_OS_WIN
    if (!environmentFlagDisabled("DD_SSH_DISABLE_WINDOWS_KEX_COMPAT")) {
        notes << QStringLiteral("Windows-safe KEX compatibility mode active: curve25519/ecdh/group14 only.");
    } else {
        notes << QStringLiteral("Windows-safe KEX compatibility mode disabled by DD_SSH_DISABLE_WINDOWS_KEX_COMPAT.");
    }
#endif

    if (environmentFlagEnabled("DD_SSH_LIBSSH_DEBUG")) {
        notes << QStringLiteral("libssh protocol debug logging enabled by DD_SSH_LIBSSH_DEBUG.");
    }

    return notes.join(QStringLiteral("\n"));
}
}
