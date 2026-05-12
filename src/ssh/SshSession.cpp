#include "SshSession.h"

#include <libssh/libssh.h>

QString SshSession::libsshVersion()
{
    const char *version = ssh_version(0);

    if (version == nullptr) {
        return QStringLiteral("unknown");
    }

    return QString::fromUtf8(version);
}
