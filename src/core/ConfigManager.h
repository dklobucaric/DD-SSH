#pragma once

#include "SessionProfile.h"

#include <QList>
#include <QString>
#include <QStringList>
#include <QtGlobal>

struct AppSettings
{
    QString terminalFontFamily = QStringLiteral("monospace");
    int terminalFontSize = 14;
    QString appTheme = QStringLiteral("system");
    bool showQuickToolbar = false;
    bool diagnosticLoggingEnabled = false;
    bool configBackupsEnabled = true;
    int maxConfigBackups = 10;
    QString doubleClickAction = QStringLiteral("open_terminal");
};


struct ConfigPreview
{
    bool exists = false;
    bool readable = false;
    bool validJson = false;
    bool isObject = false;
    bool hasProblem = false;
    QString errorMessage;
    QString filePath;
    qint64 fileSizeBytes = 0;
    int sessionCount = 0;
    int knownHostCount = 0;
    int knownHostKeyCount = 0;
    QString secretsMode;
    int secretCount = 0;
    int passwordSecretCount = 0;
    int privateKeySecretCount = 0;
    bool containsPlainSecrets = false;
    bool hasSettings = false;
    bool hasMetadata = false;
    QString configVersion;
    QStringList warnings;
};

struct ConfigInspection
{
    bool exists = false;
    bool readable = false;
    bool validJson = false;
    bool isObject = false;
    bool hasProblem = false;
    QString title;
    QString message;
    QString configFilePath;
    QString configDirectoryPath;
    QStringList backupFileNames;
};

class QJsonObject;

class ConfigManager
{
public:
    ConfigManager();

    QString configDirectoryPath() const;
    QString configFilePath() const;
    QStringList listConfigBackups() const;
    ConfigInspection inspectConfig() const;
    ConfigPreview previewConfigFile(const QString &filePath) const;
    bool createFreshConfigFromCorrupt(QString *errorMessage = nullptr, QString *movedCorruptPath = nullptr) const;
    bool restoreLatestValidBackup(QString *errorMessage = nullptr, QString *restoredBackupName = nullptr, QString *movedCorruptPath = nullptr) const;
    bool exportConfigToFile(const QString &targetPath, QString *errorMessage = nullptr) const;
    bool importConfigFromFile(const QString &sourcePath, QString *errorMessage = nullptr, QString *backupPath = nullptr) const;
    bool restoreLatestBackupReplacingCurrent(QString *errorMessage = nullptr, QString *restoredBackupName = nullptr, QString *movedCurrentPath = nullptr) const;

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
    bool writeRootObjectWithoutBackup(const QJsonObject &root, QString *errorMessage = nullptr) const;
    bool moveExistingConfigAside(const QString &suffixPrefix, QString *movedPath = nullptr, QString *errorMessage = nullptr) const;
    bool backupFileIsValidObject(const QString &absolutePath) const;
    void ensureBaseObjects(QJsonObject *root) const;
};
