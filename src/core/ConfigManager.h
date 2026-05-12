#pragma once

#include "SessionProfile.h"

#include <QList>
#include <QString>

class QJsonObject;

class ConfigManager
{
public:
    ConfigManager();

    QString configDirectoryPath() const;
    QString configFilePath() const;

    QList<SessionProfile> loadSessions(QString *errorMessage = nullptr) const;
    bool loadSessionById(
        const QString &sessionId,
        SessionProfile *session,
        QString *errorMessage = nullptr
    ) const;
    bool loadPlainSecret(
        const QString &secretId,
        QString *secretValue,
        QString *secretType = nullptr,
        QString *errorMessage = nullptr
    ) const;

    bool saveSessionWithPlainSecret(
        const SessionProfile &session,
        const QString &secretValue,
        QString *errorMessage = nullptr,
        bool *updatedExistingSession = nullptr
    ) const;

    bool deleteSession(
        const QString &sessionId,
        QString *errorMessage = nullptr,
        bool *removedUnusedSecret = nullptr,
        QString *removedSecretId = nullptr
    ) const;

    bool secureConfigFilePermissions(QString *errorMessage = nullptr) const;

    static QString makeSessionId(
        const QString &name,
        const QString &host,
        int port,
        const QString &username
    );

private:
    bool readRootObject(QJsonObject *root, QString *errorMessage = nullptr) const;
    bool writeRootObject(const QJsonObject &root, QString *errorMessage = nullptr) const;
    void ensureBaseObjects(QJsonObject *root) const;
};
