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

    bool saveSessionWithPlainSecret(
        const SessionProfile &session,
        const QString &secretValue,
        QString *errorMessage = nullptr
    ) const;

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
