#pragma once

#include <QString>

struct SshHandshakeResult
{
    bool success = false;
    QString message;
    QString serverBanner;
    QString hostKeyType;
    QString hostKeyFingerprint;
    QString error;
    int sshErrorCode = 0;
};

class SshSession
{
public:
    static QString libsshVersion();

    static SshHandshakeResult testHandshake(
        const QString &host,
        int port,
        const QString &username
    );
};
