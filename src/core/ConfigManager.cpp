#include "ConfigManager.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>

ConfigManager::ConfigManager() = default;

QString ConfigManager::configDirectoryPath() const
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);

    if (path.isEmpty()) {
        path = QDir::homePath() + QStringLiteral("/.config/DD-SSH");
    }

    return path;
}

QString ConfigManager::configFilePath() const
{
    return configDirectoryPath() + QStringLiteral("/dd-ssh.json");
}

QString ConfigManager::makeSessionId(
    const QString &name,
    const QString &host,
    int port,
    const QString &username
)
{
    QString base = name.trimmed();

    if (base.isEmpty()) {
        base = username.trimmed()
            + QStringLiteral("-")
            + host.trimmed()
            + QStringLiteral("-")
            + QString::number(port);
    }

    base = base.toLower();
    base.replace(QRegularExpression(QStringLiteral("[^a-z0-9]+")), QStringLiteral("-"));
    base.replace(QRegularExpression(QStringLiteral("^-+|-+$")), QString());

    if (base.isEmpty()) {
        base = QStringLiteral("session-") + QString::number(QDateTime::currentSecsSinceEpoch());
    }

    return base;
}

bool ConfigManager::readRootObject(QJsonObject *root, QString *errorMessage) const
{
    if (root == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Internal error: missing root object pointer.");
        }

        return false;
    }

    *root = QJsonObject();

    QFile file(configFilePath());

    if (!file.exists()) {
        *root = QJsonObject();
        return true;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Could not read config file: ") + file.errorString();
        }

        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Could not parse config JSON: ") + parseError.errorString();
        }

        return false;
    }

    if (!document.isObject()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Config file is not a JSON object.");
        }

        return false;
    }

    *root = document.object();
    return true;
}

void ConfigManager::ensureBaseObjects(QJsonObject *root) const
{
    if (root == nullptr) {
        return;
    }

    QJsonObject app = root->value(QStringLiteral("app")).toObject();

    if (!app.contains(QStringLiteral("name"))) {
        app.insert(QStringLiteral("name"), QStringLiteral("DD-SSH"));
    }

    if (!app.contains(QStringLiteral("config_version"))) {
        app.insert(QStringLiteral("config_version"), 1);
    }

    root->insert(QStringLiteral("app"), app);

    if (!root->value(QStringLiteral("settings")).isObject()) {
        root->insert(QStringLiteral("settings"), QJsonObject());
    }

    if (!root->value(QStringLiteral("groups")).isArray()) {
        root->insert(QStringLiteral("groups"), QJsonArray());
    }

    if (!root->value(QStringLiteral("sessions")).isArray()) {
        root->insert(QStringLiteral("sessions"), QJsonArray());
    }

    if (!root->value(QStringLiteral("known_hosts")).isObject()) {
        root->insert(QStringLiteral("known_hosts"), QJsonObject());
    }

    QJsonObject secrets = root->value(QStringLiteral("secrets")).toObject();

    if (!secrets.contains(QStringLiteral("mode"))) {
        secrets.insert(QStringLiteral("mode"), QStringLiteral("plain-v1"));
    }

    if (!secrets.value(QStringLiteral("items")).isObject()) {
        secrets.insert(QStringLiteral("items"), QJsonObject());
    }

    root->insert(QStringLiteral("secrets"), secrets);

    QJsonObject metadata = root->value(QStringLiteral("metadata")).toObject();

    if (!metadata.contains(QStringLiteral("created_by"))) {
        metadata.insert(QStringLiteral("created_by"), QStringLiteral("DD-SSH"));
    }

    root->insert(QStringLiteral("metadata"), metadata);
}

bool ConfigManager::writeRootObject(const QJsonObject &root, QString *errorMessage) const
{
    QDir directory(configDirectoryPath());

    if (!directory.exists() && !directory.mkpath(QStringLiteral("."))) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Could not create config directory: ") + directory.absolutePath();
        }

        return false;
    }

    QSaveFile saveFile(configFilePath());

    if (!saveFile.open(QIODevice::WriteOnly)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Could not open config file for writing: ") + saveFile.errorString();
        }

        return false;
    }

    const QJsonDocument document(root);
    saveFile.write(document.toJson(QJsonDocument::Indented));

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

QList<SessionProfile> ConfigManager::loadSessions(QString *errorMessage) const
{
    QList<SessionProfile> sessions;
    QJsonObject root;

    if (!readRootObject(&root, errorMessage)) {
        return sessions;
    }

    const QJsonArray sessionArray = root.value(QStringLiteral("sessions")).toArray();

    for (const QJsonValue &value : sessionArray) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject object = value.toObject();
        const QJsonObject auth = object.value(QStringLiteral("auth")).toObject();

        SessionProfile session;
        session.id = object.value(QStringLiteral("id")).toString();
        session.name = object.value(QStringLiteral("name")).toString();
        session.group = object.value(QStringLiteral("group")).toString();
        session.host = object.value(QStringLiteral("host")).toString();
        session.port = object.value(QStringLiteral("port")).toInt(22);
        session.username = object.value(QStringLiteral("username")).toString();

        const QString authType = auth.value(QStringLiteral("type")).toString(QStringLiteral("password"));

        if (authType == QStringLiteral("key")) {
            session.authType = SessionProfile::AuthType::PrivateKey;
            session.keyRef = auth.value(QStringLiteral("key_ref")).toString();
        } else {
            session.authType = SessionProfile::AuthType::Password;
            session.secretRef = auth.value(QStringLiteral("secret_ref")).toString();
        }

        if (session.id.isEmpty() || session.name.isEmpty() || session.host.isEmpty() || session.username.isEmpty()) {
            continue;
        }

        sessions.append(session);
    }

    return sessions;
}

bool ConfigManager::loadSessionById(
    const QString &sessionId,
    SessionProfile *session,
    QString *errorMessage
) const
{
    if (errorMessage != nullptr) {
        *errorMessage = QString();
    }

    if (session == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Internal error: missing session pointer.");
        }

        return false;
    }

    const QList<SessionProfile> sessions = loadSessions(errorMessage);

    if (errorMessage != nullptr && !errorMessage->isEmpty()) {
        return false;
    }

    for (const SessionProfile &candidate : sessions) {
        if (candidate.id == sessionId) {
            *session = candidate;
            return true;
        }
    }

    if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("Saved session not found: ") + sessionId;
    }

    return false;
}

bool ConfigManager::loadPlainSecret(
    const QString &secretId,
    QString *secretValue,
    QString *secretType,
    QString *errorMessage
) const
{
    if (errorMessage != nullptr) {
        *errorMessage = QString();
    }

    if (secretValue == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Internal error: missing secret value pointer.");
        }

        return false;
    }

    *secretValue = QString();

    if (secretType != nullptr) {
        *secretType = QString();
    }

    const QString trimmedSecretId = secretId.trimmed();

    if (trimmedSecretId.isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Secret reference is empty.");
        }

        return false;
    }

    QJsonObject root;

    if (!readRootObject(&root, errorMessage)) {
        return false;
    }

    const QJsonObject secrets = root.value(QStringLiteral("secrets")).toObject();
    const QString mode = secrets.value(QStringLiteral("mode")).toString();

    if (mode != QStringLiteral("plain-v1")) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Unsupported secrets mode: ") + mode;
        }

        return false;
    }

    const QJsonObject items = secrets.value(QStringLiteral("items")).toObject();

    if (!items.contains(trimmedSecretId)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Secret not found: ") + trimmedSecretId;
        }

        return false;
    }

    const QJsonObject secretObject = items.value(trimmedSecretId).toObject();
    const QString loadedType = secretObject.value(QStringLiteral("type")).toString();

    if (secretType != nullptr) {
        *secretType = loadedType;
    }

    *secretValue = secretObject.value(QStringLiteral("value")).toString();
    return true;
}

bool ConfigManager::saveSessionWithPlainSecret(
    const SessionProfile &session,
    const QString &secretValue,
    QString *errorMessage
) const
{
    if (session.id.trimmed().isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Session id is empty.");
        }

        return false;
    }

    QJsonObject root;

    if (!readRootObject(&root, errorMessage)) {
        return false;
    }

    ensureBaseObjects(&root);

    const QString nowUtc = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    QJsonArray groups = root.value(QStringLiteral("groups")).toArray();
    const QString groupName = session.group.trimmed();

    if (!groupName.isEmpty()) {
        bool groupExists = false;

        for (const QJsonValue &value : groups) {
            const QJsonObject group = value.toObject();

            if (group.value(QStringLiteral("name")).toString().compare(groupName, Qt::CaseInsensitive) == 0) {
                groupExists = true;
                break;
            }
        }

        if (!groupExists) {
            QString groupId = groupName.toLower();
            groupId.replace(QRegularExpression(QStringLiteral("[^a-z0-9]+")), QStringLiteral("-"));
            groupId.replace(QRegularExpression(QStringLiteral("^-+|-+$")), QString());

            if (groupId.isEmpty()) {
                groupId = QStringLiteral("default");
            }

            QJsonObject groupObject;
            groupObject.insert(QStringLiteral("id"), groupId);
            groupObject.insert(QStringLiteral("name"), groupName);
            groups.append(groupObject);
        }
    }

    root.insert(QStringLiteral("groups"), groups);

    QJsonObject authObject;

    const QString secretId = session.authType == SessionProfile::AuthType::PrivateKey
        ? QStringLiteral("secret-") + session.id + QStringLiteral("-key")
        : QStringLiteral("secret-") + session.id + QStringLiteral("-password");

    if (session.authType == SessionProfile::AuthType::PrivateKey) {
        authObject.insert(QStringLiteral("type"), QStringLiteral("key"));
        authObject.insert(QStringLiteral("key_ref"), secretId);
    } else {
        authObject.insert(QStringLiteral("type"), QStringLiteral("password"));
        authObject.insert(QStringLiteral("secret_ref"), secretId);
    }

    QJsonObject sessionObject;
    sessionObject.insert(QStringLiteral("id"), session.id);
    sessionObject.insert(QStringLiteral("name"), session.name.trimmed());
    sessionObject.insert(QStringLiteral("group"), groupName);
    sessionObject.insert(QStringLiteral("host"), session.host.trimmed());
    sessionObject.insert(QStringLiteral("port"), session.port);
    sessionObject.insert(QStringLiteral("username"), session.username.trimmed());
    sessionObject.insert(QStringLiteral("auth"), authObject);
    sessionObject.insert(QStringLiteral("last_successful_auth"), nowUtc);

    QJsonArray sessions = root.value(QStringLiteral("sessions")).toArray();
    bool replaced = false;

    for (int i = 0; i < sessions.size(); ++i) {
        const QJsonObject existing = sessions.at(i).toObject();

        if (existing.value(QStringLiteral("id")).toString() == session.id) {
            sessionObject.insert(
                QStringLiteral("created_at"),
                existing.value(QStringLiteral("created_at")).toString(nowUtc)
            );
            sessions.replace(i, sessionObject);
            replaced = true;
            break;
        }
    }

    if (!replaced) {
        sessionObject.insert(QStringLiteral("created_at"), nowUtc);
        sessions.append(sessionObject);
    }

    root.insert(QStringLiteral("sessions"), sessions);

    QJsonObject secrets = root.value(QStringLiteral("secrets")).toObject();
    secrets.insert(QStringLiteral("mode"), QStringLiteral("plain-v1"));

    QJsonObject items = secrets.value(QStringLiteral("items")).toObject();

    QJsonObject secretObject;
    secretObject.insert(
        QStringLiteral("type"),
        session.authType == SessionProfile::AuthType::PrivateKey
            ? QStringLiteral("private_key")
            : QStringLiteral("password")
    );
    secretObject.insert(QStringLiteral("value"), secretValue);
    secretObject.insert(QStringLiteral("updated_at"), nowUtc);

    items.insert(secretId, secretObject);
    secrets.insert(QStringLiteral("items"), items);
    root.insert(QStringLiteral("secrets"), secrets);

    QJsonObject metadata = root.value(QStringLiteral("metadata")).toObject();
    metadata.insert(QStringLiteral("created_by"), QStringLiteral("DD-SSH"));
    metadata.insert(QStringLiteral("last_modified"), nowUtc);
    root.insert(QStringLiteral("metadata"), metadata);

    return writeRootObject(root, errorMessage);
}
