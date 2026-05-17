#pragma once

#include <QString>

class QJsonObject;

class KnownHostsManager
{
public:
    enum class HostStatus
    {
        Unknown,
        Trusted,
        Changed
    };

    struct CheckResult
    {
        HostStatus status = HostStatus::Unknown;
        QString hostKey;
        QString storedKeyType;
        QString storedFingerprint;
    };

    KnownHostsManager();

    QString configFilePath() const;

    CheckResult checkHost(
        const QString &host,
        int port,
        const QString &keyType,
        const QString &fingerprint
    ) const;

    bool trustHost(
        const QString &host,
        int port,
        const QString &keyType,
        const QString &fingerprint,
        QString *errorMessage = nullptr
    ) const;

private:
    QString makeHostKey(const QString &host, int port) const;
    QString configDirectoryPath() const;
    bool createConfigBackupIfNeeded(const QJsonObject &root, QString *errorMessage = nullptr) const;
    bool pruneConfigBackups(int maxBackups, QString *errorMessage = nullptr) const;
};
