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

enum class SshAuthMethod
{
    Password,
    PrivateKey
};

struct SshAuthResult
{
    bool success = false;
    QString message;
    QString error;
    int sshErrorCode = 0;
    int authReturnCode = 0;
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

    static SshAuthResult testAuthentication(
        const QString &host,
        int port,
        const QString &username,
        SshAuthMethod authMethod,
        const QString &password,
        const QString &privateKeyPath
    );
};
