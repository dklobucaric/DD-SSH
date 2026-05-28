#pragma once

#include "ssh/SshSession.h"

#include <QList>
#include <QString>
#include <QtGlobal>

struct SftpRemoteEntry
{
    QString name;
    QString type;
    quint64 sizeBytes = 0;
    QString modifiedTime;
    QString permissions;
};

struct SftpProbeResult
{
    bool success = false;
    QString message;
    QString error;
    int sshErrorCode = 0;
    int authReturnCode = 0;
    int sftpErrorCode = 0;
    bool hostKeyVerificationAttempted = false;
    bool hostKeyVerified = false;
    QString hostKeyType;
    QString hostKeyFingerprint;
    QString remotePath;
    QList<SftpRemoteEntry> entries;
};

class SftpProbe
{
public:
    static SftpProbeResult listRemoteDirectory(
        const QString &host,
        int port,
        const QString &username,
        SshAuthMethod authMethod,
        const QString &secretValue,
        const SshHostKeyExpectation &hostKeyExpectation,
        const QString &remotePath = QStringLiteral(".")
    );
};
