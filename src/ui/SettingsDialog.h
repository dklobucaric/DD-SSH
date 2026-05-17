#pragma once

#include "core/ConfigManager.h"

#include <QDialog>
#include <QString>

class QCheckBox;
class QLabel;
class QLineEdit;
class QComboBox;
class QSpinBox;

class SettingsDialog : public QDialog
{
public:
    explicit SettingsDialog(
        const AppSettings &settings,
        const QString &configFilePath,
        QWidget *parent = nullptr
    );

    AppSettings settings() const;

private:
    QString m_configFilePath;

    QLineEdit *m_configPathEdit = nullptr;
    QComboBox *m_appThemeCombo = nullptr;
    QLineEdit *m_terminalFontFamilyEdit = nullptr;
    QSpinBox *m_terminalFontSizeSpin = nullptr;
    QCheckBox *m_configBackupsCheck = nullptr;
    QSpinBox *m_maxBackupsSpin = nullptr;
    QLabel *m_doubleClickLabel = nullptr;
};
