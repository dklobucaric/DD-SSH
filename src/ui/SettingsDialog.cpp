#include "SettingsDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QUrl>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(
    const AppSettings &settings,
    const QString &configFilePath,
    QWidget *parent
)
    : QDialog(parent)
    , m_configFilePath(configFilePath)
{
    setWindowTitle(QStringLiteral("DD-SSH Settings"));
    setModal(true);
    setSizeGripEnabled(true);
    setMinimumSize(900, 820);
    resize(940, 860);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(14, 14, 14, 14);
    mainLayout->setSpacing(12);
    mainLayout->setSizeConstraint(QLayout::SetMinimumSize);

    auto *titleLabel = new QLabel(QStringLiteral("DD-SSH settings"), this);
    titleLabel->setStyleSheet(QStringLiteral("font-weight: bold;"));
    mainLayout->addWidget(titleLabel);

    auto *introLabel = new QLabel(
        QStringLiteral("These settings are stored in dd-ssh.json under the top-level settings block. "
                       "App theme changes apply immediately after saving; terminal font changes apply to newly opened terminals."),
        this
    );
    introLabel->setWordWrap(true);
    mainLayout->addWidget(introLabel);

    auto *generalGroup = new QGroupBox(QStringLiteral("General"), this);
    auto *generalLayout = new QFormLayout(generalGroup);
    generalLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    auto *configPathRow = new QWidget(generalGroup);
    auto *configPathLayout = new QHBoxLayout(configPathRow);
    configPathLayout->setContentsMargins(0, 0, 0, 0);

    m_configPathEdit = new QLineEdit(configPathRow);
    m_configPathEdit->setMinimumWidth(520);
    m_configPathEdit->setText(m_configFilePath);
    m_configPathEdit->setReadOnly(true);

    auto *openConfigFolderButton = new QPushButton(QStringLiteral("Open folder"), configPathRow);

    configPathLayout->addWidget(m_configPathEdit, 1);
    configPathLayout->addWidget(openConfigFolderButton);
    generalLayout->addRow(QStringLiteral("Config file:"), configPathRow);

    m_doubleClickLabel = new QLabel(QStringLiteral("Open terminal"), generalGroup);
    generalLayout->addRow(QStringLiteral("Double-click session:"), m_doubleClickLabel);

    m_showQuickToolbarCheck = new QCheckBox(QStringLiteral("Show quick action toolbar"), generalGroup);
    m_showQuickToolbarCheck->setChecked(settings.showQuickToolbar);
    m_showQuickToolbarCheck->setToolTip(QStringLiteral("Show the optional toolbar with New Session, Connect, Multi-Exec, and Settings shortcuts."));
    generalLayout->addRow(QStringLiteral("Interface:"), m_showQuickToolbarCheck);

    auto *toolbarNote = new QLabel(
        QStringLiteral("When hidden, all actions remain available from the File, Session, Tools, and Help menus."),
        generalGroup
    );
    toolbarNote->setWordWrap(true);
    generalLayout->addRow(QString(), toolbarNote);

    generalGroup->setMinimumHeight(145);
    mainLayout->addWidget(generalGroup);

    auto *diagnosticsGroup = new QGroupBox(QStringLiteral("Diagnostics"), this);
    auto *diagnosticsLayout = new QFormLayout(diagnosticsGroup);
    diagnosticsLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    m_diagnosticLoggingCheck = new QCheckBox(QStringLiteral("Enable diagnostic logging"), diagnosticsGroup);
    m_diagnosticLoggingCheck->setChecked(settings.diagnosticLoggingEnabled);
    m_diagnosticLoggingCheck->setToolTip(QStringLiteral("Write detailed DD-SSH diagnostic events to a standard per-user log folder. Secrets, terminal input, and terminal output are never logged."));
    diagnosticsLayout->addRow(QStringLiteral("Logging:"), m_diagnosticLoggingCheck);

    auto *diagnosticsNote = new QLabel(
        QStringLiteral("Logging is off by default and intended for on-demand debugging. Use Help → Open Log Folder to view or send logs when troubleshooting."),
        diagnosticsGroup
    );
    diagnosticsNote->setWordWrap(true);
    diagnosticsLayout->addRow(QString(), diagnosticsNote);

    diagnosticsGroup->setMinimumHeight(120);
    mainLayout->addWidget(diagnosticsGroup);

    auto *appearanceGroup = new QGroupBox(QStringLiteral("Appearance"), this);
    auto *appearanceLayout = new QFormLayout(appearanceGroup);
    appearanceLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    m_appThemeCombo = new QComboBox(appearanceGroup);
    m_appThemeCombo->addItem(QStringLiteral("System default"), QStringLiteral("system"));
    m_appThemeCombo->addItem(QStringLiteral("Light"), QStringLiteral("light"));
    m_appThemeCombo->addItem(QStringLiteral("Dark"), QStringLiteral("dark"));

    const QString normalizedTheme = settings.appTheme.trimmed().toLower();
    const int themeIndex = m_appThemeCombo->findData(
        (normalizedTheme == QStringLiteral("light") || normalizedTheme == QStringLiteral("dark"))
            ? normalizedTheme
            : QStringLiteral("system")
    );

    if (themeIndex >= 0) {
        m_appThemeCombo->setCurrentIndex(themeIndex);
    }

    appearanceLayout->addRow(QStringLiteral("App theme:"), m_appThemeCombo);

    auto *appearanceNote = new QLabel(
        QStringLiteral("This changes the Qt app chrome only. The xterm.js terminal stays on its current dark terminal theme for now."),
        appearanceGroup
    );
    appearanceNote->setWordWrap(true);
    appearanceLayout->addRow(QString(), appearanceNote);

    appearanceGroup->setMinimumHeight(125);
    mainLayout->addWidget(appearanceGroup);

    auto *terminalGroup = new QGroupBox(QStringLiteral("Terminal"), this);
    auto *terminalLayout = new QFormLayout(terminalGroup);
    terminalLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    m_terminalFontFamilyEdit = new QLineEdit(terminalGroup);
    m_terminalFontFamilyEdit->setText(settings.terminalFontFamily.trimmed().isEmpty()
        ? QStringLiteral("monospace")
        : settings.terminalFontFamily.trimmed());
    m_terminalFontFamilyEdit->setPlaceholderText(QStringLiteral("monospace"));
    terminalLayout->addRow(QStringLiteral("Font family:"), m_terminalFontFamilyEdit);

    m_terminalFontSizeSpin = new QSpinBox(terminalGroup);
    m_terminalFontSizeSpin->setRange(8, 36);
    m_terminalFontSizeSpin->setValue(settings.terminalFontSize);
    terminalLayout->addRow(QStringLiteral("Font size:"), m_terminalFontSizeSpin);

    auto *terminalNote = new QLabel(
        QStringLiteral("Font changes apply to new xterm.js terminal tabs. Already-open tabs are not changed."),
        terminalGroup
    );
    terminalNote->setWordWrap(true);
    terminalLayout->addRow(QString(), terminalNote);

    terminalGroup->setMinimumHeight(125);
    mainLayout->addWidget(terminalGroup);

    auto *configSafetyGroup = new QGroupBox(QStringLiteral("Config safety"), this);
    auto *configSafetyLayout = new QFormLayout(configSafetyGroup);
    configSafetyLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    m_configBackupsCheck = new QCheckBox(QStringLiteral("Enable config backups before save"), configSafetyGroup);
    m_configBackupsCheck->setChecked(settings.configBackupsEnabled);
    configSafetyLayout->addRow(QStringLiteral("Backups:"), m_configBackupsCheck);

    m_maxBackupsSpin = new QSpinBox(configSafetyGroup);
    m_maxBackupsSpin->setRange(1, 50);
    m_maxBackupsSpin->setValue(settings.maxConfigBackups);
    configSafetyLayout->addRow(QStringLiteral("Keep last:"), m_maxBackupsSpin);

    auto *backupNote = new QLabel(
        QStringLiteral("DD-SSH now creates rotating config backups before saving when backups are enabled."),
        configSafetyGroup
    );
    backupNote->setWordWrap(true);
    configSafetyLayout->addRow(QString(), backupNote);

    configSafetyGroup->setMinimumHeight(130);
    mainLayout->addWidget(configSafetyGroup);

    auto *warningLabel = new QLabel(
        QStringLiteral("Security note: DD-SSH currently uses secrets.mode = plain-v1. "
                       "Saved passwords and private keys are portable but stored in plaintext in dd-ssh.json."),
        this
    );
    warningLabel->setWordWrap(true);
    warningLabel->setStyleSheet(QStringLiteral("color: #b36b00;"));
    mainLayout->addWidget(warningLabel);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this
    );
    mainLayout->addWidget(buttons);

    connect(openConfigFolderButton, &QPushButton::clicked, this, [this]() {
        const QFileInfo info(m_configFilePath);
        const QString folder = info.absolutePath();

        if (folder.trimmed().isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("Could not open config folder"), QStringLiteral("Config folder path is empty."));
            return;
        }

        QDesktopServices::openUrl(QUrl::fromLocalFile(folder));
    });

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        if (m_terminalFontFamilyEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("Missing font family"), QStringLiteral("Please enter a terminal font family or use 'monospace'."));
            m_terminalFontFamilyEdit->setFocus();
            return;
        }

        accept();
    });
}

AppSettings SettingsDialog::settings() const
{
    AppSettings result;
    result.appTheme = QStringLiteral("system");

    if (m_appThemeCombo != nullptr) {
        const QString selectedTheme = m_appThemeCombo->currentData().toString().trimmed();

        if (!selectedTheme.isEmpty()) {
            result.appTheme = selectedTheme;
        }
    }
    result.terminalFontFamily = m_terminalFontFamilyEdit->text().trimmed().isEmpty()
        ? QStringLiteral("monospace")
        : m_terminalFontFamilyEdit->text().trimmed();
    result.terminalFontSize = m_terminalFontSizeSpin->value();
    result.showQuickToolbar = (m_showQuickToolbarCheck != nullptr) && m_showQuickToolbarCheck->isChecked();
    result.diagnosticLoggingEnabled = (m_diagnosticLoggingCheck != nullptr) && m_diagnosticLoggingCheck->isChecked();
    result.configBackupsEnabled = m_configBackupsCheck->isChecked();
    result.maxConfigBackups = m_maxBackupsSpin->value();
    result.doubleClickAction = QStringLiteral("open_terminal");
    return result;
}
