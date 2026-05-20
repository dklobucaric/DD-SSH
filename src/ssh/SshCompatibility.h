#pragma once

#include <QString>

#include <libssh/libssh.h>

namespace SshCompatibility
{
    int defaultLogVerbosity();
    void applySessionCompatibility(ssh_session session);
    QString activeCompatibilityNotes();
}
