#pragma once

#include "ssh/SshSession.h"

#include <QList>
#include <QString>
#include <QtGlobal>

#include <functional>

struct SftpRemoteEntry
{
    QString name;
    QString type;
    quint64 sizeBytes = 0;
    QString modifiedTime;
    QString permissions;
};


struct SftpDownloadResult
{
    bool success = false;
    bool cancelled = false;
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
    QString localPath;
    quint64 bytesTransferred = 0;
    quint64 totalBytes = 0;
};

using SftpDownloadProgressCallback = std::function<bool(quint64 bytesTransferred, quint64 totalBytes, const QString &message)>;

struct SftpUploadResult
{
    bool success = false;
    bool cancelled = false;
    bool remoteAlreadyExists = false;
    bool remoteTargetIsDirectory = false;
    QString message;
    QString error;
    int sshErrorCode = 0;
    int authReturnCode = 0;
    int sftpErrorCode = 0;
    bool hostKeyVerificationAttempted = false;
    bool hostKeyVerified = false;
    QString hostKeyType;
    QString hostKeyFingerprint;
    QString localPath;
    QString remotePath;
    quint64 bytesTransferred = 0;
    quint64 totalBytes = 0;
};

using SftpUploadProgressCallback = std::function<bool(quint64 bytesTransferred, quint64 totalBytes, const QString &message)>;

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

    static SftpDownloadResult downloadRemoteFile(
        const QString &host,
        int port,
        const QString &username,
        SshAuthMethod authMethod,
        const QString &secretValue,
        const SshHostKeyExpectation &hostKeyExpectation,
        const QString &remotePath,
        const QString &localPath,
        SftpDownloadProgressCallback progressCallback = {}
    );

    static SftpUploadResult uploadLocalFile(
        const QString &host,
        int port,
        const QString &username,
        SshAuthMethod authMethod,
        const QString &secretValue,
        const SshHostKeyExpectation &hostKeyExpectation,
        const QString &localPath,
        const QString &remotePath,
        bool allowOverwrite,
        SftpUploadProgressCallback progressCallback = {}
    );
};
