#include "KnownHostsManager.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>

KnownHostsManager::KnownHostsManager() = default;

QString KnownHostsManager::configDirectoryPath() const
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);

    if (path.isEmpty()) {
        path = QDir::homePath() + QStringLiteral("/.config/DD-SSH");
    }

    return path;
}

QString KnownHostsManager::configFilePath() const
{
    return configDirectoryPath() + QStringLiteral("/dd-ssh.json");
}

QString KnownHostsManager::makeHostKey(const QString &host, int port) const
{
    return host.trimmed() + QStringLiteral(":") + QString::number(port);
}

KnownHostsManager::CheckResult KnownHostsManager::checkHost(
    const QString &host,
    int port,
    const QString &keyType,
    const QString &fingerprint
) const
{
    CheckResult result;
    result.hostKey = makeHostKey(host, port);

    QFile file(configFilePath());

    if (!file.exists()) {
        result.status = HostStatus::Unknown;
        return result;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        result.status = HostStatus::Unknown;
        return result;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!document.isObject()) {
        result.status = HostStatus::Unknown;
        return result;
    }

    const QJsonObject root = document.object();
    const QJsonObject knownHosts = root.value(QStringLiteral("known_hosts")).toObject();

    if (!knownHosts.contains(result.hostKey)) {
        result.status = HostStatus::Unknown;
        return result;
    }

    const QJsonObject storedHost = knownHosts.value(result.hostKey).toObject();

    result.storedKeyType = storedHost.value(QStringLiteral("algorithm")).toString();
    result.storedFingerprint = storedHost.value(QStringLiteral("fingerprint")).toString();

    if (result.storedKeyType == keyType && result.storedFingerprint == fingerprint) {
        result.status = HostStatus::Trusted;
        return result;
    }

    result.status = HostStatus::Changed;
    return result;
}

bool KnownHostsManager::trustHost(
    const QString &host,
    int port,
    const QString &keyType,
    const QString &fingerprint,
    QString *errorMessage
) const
{
    QDir directory(configDirectoryPath());

    if (!directory.exists() && !directory.mkpath(QStringLiteral("."))) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Could not create config directory: ") + directory.absolutePath();
        }

        return false;
    }

    QJsonObject root;
    QFile existingFile(configFilePath());

    if (existingFile.exists()) {
        if (!existingFile.open(QIODevice::ReadOnly)) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("Could not read existing config file: ") + existingFile.errorString();
            }

            return false;
        }

        const QJsonDocument existingDocument = QJsonDocument::fromJson(existingFile.readAll());
        existingFile.close();

        if (existingDocument.isObject()) {
            root = existingDocument.object();
        }
    }

    QJsonObject app = root.value(QStringLiteral("app")).toObject();

    if (!app.contains(QStringLiteral("name"))) {
        app.insert(QStringLiteral("name"), QStringLiteral("DD-SSH"));
    }

    if (!app.contains(QStringLiteral("config_version"))) {
        app.insert(QStringLiteral("config_version"), 1);
    }

    root.insert(QStringLiteral("app"), app);

    QJsonObject knownHosts = root.value(QStringLiteral("known_hosts")).toObject();

    const QString hostKey = makeHostKey(host, port);
    QJsonObject existingHost = knownHosts.value(hostKey).toObject();

    const QString nowUtc = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    QJsonObject hostObject;
    hostObject.insert(QStringLiteral("algorithm"), keyType);
    hostObject.insert(QStringLiteral("fingerprint"), fingerprint);

    if (existingHost.contains(QStringLiteral("first_seen"))) {
        hostObject.insert(QStringLiteral("first_seen"), existingHost.value(QStringLiteral("first_seen")).toString());
    } else {
        hostObject.insert(QStringLiteral("first_seen"), nowUtc);
    }

    hostObject.insert(QStringLiteral("last_seen"), nowUtc);

    knownHosts.insert(hostKey, hostObject);
    root.insert(QStringLiteral("known_hosts"), knownHosts);

    QJsonObject metadata = root.value(QStringLiteral("metadata")).toObject();
    metadata.insert(QStringLiteral("last_modified"), nowUtc);
    root.insert(QStringLiteral("metadata"), metadata);

    QSaveFile saveFile(configFilePath());

    if (!saveFile.open(QIODevice::WriteOnly)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Could not open config file for writing: ") + saveFile.errorString();
        }

        return false;
    }

    const QJsonDocument outputDocument(root);
    saveFile.write(outputDocument.toJson(QJsonDocument::Indented));

    if (!saveFile.commit()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Could not save config file: ") + saveFile.errorString();
        }

        return false;
    }

    QFile::setPermissions(
        configFilePath(),
        QFileDevice::ReadOwner | QFileDevice::WriteOwner
    );

    return true;
}
