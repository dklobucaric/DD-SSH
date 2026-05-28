#include "AppLogger.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QStringList>
#include <QTextStream>

namespace {
QMutex &loggerMutex()
{
    static QMutex mutex;
    return mutex;
}

bool &loggerEnabled()
{
    static bool enabled = false;
    return enabled;
}

QString levelToString(AppLogger::Level level)
{
    switch (level) {
    case AppLogger::Level::Info:
        return QStringLiteral("INFO");
    case AppLogger::Level::Warn:
        return QStringLiteral("WARN");
    case AppLogger::Level::Error:
        return QStringLiteral("ERROR");
    }

    return QStringLiteral("INFO");
}

QString fallbackLocalDataPath()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);

    if (path.isEmpty()) {
        path = QDir::homePath() + QStringLiteral("/.local/share/DD-SSH");
    }

    return path;
}

QString platformLogDirectoryPath()
{
#if defined(Q_OS_WIN)
    const QString localAppData = QString::fromLocal8Bit(qgetenv("LOCALAPPDATA")).trimmed();

    if (!localAppData.isEmpty()) {
        return QDir(localAppData).filePath(QStringLiteral("DD-SSH/logs"));
    }

    return QDir(fallbackLocalDataPath()).filePath(QStringLiteral("logs"));
#elif defined(Q_OS_MACOS) || defined(Q_OS_DARWIN)
    return QDir(QDir::homePath()).filePath(QStringLiteral("Library/Logs/DD-SSH"));
#else
    return QDir(QDir::homePath()).filePath(QStringLiteral(".local/state/DD-SSH/logs"));
#endif
}

bool ensureLogDirectoryUnlocked(QString *errorMessage)
{
    if (errorMessage != nullptr) {
        *errorMessage = QString();
    }

    QDir directory(AppLogger::logDirectoryPath());

    if (directory.exists()) {
        return true;
    }

    if (!directory.mkpath(QStringLiteral("."))) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Could not create DD-SSH log directory: ") + directory.absolutePath();
        }

        return false;
    }

    return true;
}

void writeLineUnlocked(AppLogger::Level level, const QString &message)
{
    QString directoryError;

    if (!ensureLogDirectoryUnlocked(&directoryError)) {
        return;
    }

    QFile file(AppLogger::currentLogFilePath());

    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    stream << QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"))
           << QStringLiteral(" - ")
           << levelToString(level)
           << QStringLiteral(" - ")
           << message
           << Qt::endl;
}
}

void AppLogger::setEnabled(bool enabled)
{
    QMutexLocker locker(&loggerMutex());

    if (loggerEnabled() == enabled) {
        return;
    }

    if (enabled) {
        loggerEnabled() = true;
        writeLineUnlocked(Level::Info, QStringLiteral("Diagnostic logging enabled"));
        return;
    }

    writeLineUnlocked(Level::Info, QStringLiteral("Diagnostic logging disabled"));
    loggerEnabled() = false;
}

bool AppLogger::isEnabled()
{
    QMutexLocker locker(&loggerMutex());
    return loggerEnabled();
}

QString AppLogger::logDirectoryPath()
{
    return QDir::cleanPath(platformLogDirectoryPath());
}

QString AppLogger::currentLogFilePath()
{
    return QDir(logDirectoryPath()).filePath(
        QDate::currentDate().toString(QStringLiteral("yyyyMMdd")) + QStringLiteral(".log")
    );
}

bool AppLogger::ensureLogDirectory(QString *errorMessage)
{
    QMutexLocker locker(&loggerMutex());
    return ensureLogDirectoryUnlocked(errorMessage);
}

void AppLogger::info(const QString &message)
{
    log(Level::Info, message);
}

void AppLogger::warn(const QString &message)
{
    log(Level::Warn, message);
}

void AppLogger::error(const QString &message)
{
    log(Level::Error, message);
}

void AppLogger::log(Level level, const QString &message)
{
    QMutexLocker locker(&loggerMutex());

    if (!loggerEnabled()) {
        return;
    }

    writeLineUnlocked(level, message);
}
