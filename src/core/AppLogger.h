#pragma once

#include <QString>

class AppLogger
{
public:
    enum class Level
    {
        Info,
        Warn,
        Error
    };

    static void setEnabled(bool enabled);
    static bool isEnabled();

    static QString logDirectoryPath();
    static QString currentLogFilePath();
    static bool ensureLogDirectory(QString *errorMessage = nullptr);

    static void info(const QString &message);
    static void warn(const QString &message);
    static void error(const QString &message);
    static void log(Level level, const QString &message);
};
