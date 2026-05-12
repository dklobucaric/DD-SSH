#pragma once

#include <QString>

struct SessionProfile
{
    enum class AuthType
    {
        Password,
        PrivateKey
    };

    QString id;
    QString name;
    QString group;
    QString host;
    int port = 22;
    QString username;
    AuthType authType = AuthType::Password;
    QString secretRef;
    QString keyRef;
};
