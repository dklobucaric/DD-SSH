#pragma once

#include "SessionProfile.h"

#include <QList>
#include <QString>

struct AppSettings
{
    QString terminalFontFamily = QStringLiteral("monospace");
    int terminalFontSize = 14;
    QString appTheme = QStringLiteral("system");
    bool configBackupsEnabled = true;
    int maxConfigBackups = 10;
    QString doubleClickAction = QStringLiteral("open_terminal");
};

class QJsonObject;

class ConfigManager
{
public:
    ConfigManager();

    QString configDirectoryPath() const;
    QString configFilePath() const;

    AppSettings loadSettings(QString *errorMessage = nullptr) const;
    bool saveSettings(const AppSettings &settings, QString *errorMessage = nullptr) const;

    QList<SessionProfile> loadSessions(QString *errorMessage = nullptr) const;
    QList<SessionProfile> findSessionsByTarget(
        const QString &host,
        int port,
        const QString &username,
        const QString &excludedSessionId = QString(),
        QString *errorMessage = nullptr
    ) const;
    QString makeUniqueSessionId(
        const QString &desiredSessionId,
        const QString &excludedSessionId = QString(),
        QString *errorMessage = nullptr
    ) const;
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

    bool updateSessionWithOptionalPlainSecret(
        const QString &originalSessionId,
        const SessionProfile &session,
        const QString &secretValue,
        bool replaceSecret,
        QString *errorMessage = nullptr,
        bool *changedSessionId = nullptr
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
    bool createConfigBackupIfNeeded(const QJsonObject &root, QString *errorMessage = nullptr) const;
    bool pruneConfigBackups(int maxBackups, QString *errorMessage = nullptr) const;
    void ensureBaseObjects(QJsonObject *root) const;
};
