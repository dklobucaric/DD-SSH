#include "SettingsDialog.h"

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QSpinBox>
#include <QStackedWidget>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
constexpr int LabelColumnWidth = 170;

QLabel *makePageTitle(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 15px;"));
    return label;
}

QLabel *makeDescription(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setStyleSheet(QStringLiteral("color: palette(mid);"));
    return label;
}

QToolButton *makeInfoButton(const QString &tooltip, QWidget *parent)
{
    auto *button = new QToolButton(parent);
    button->setText(QStringLiteral("i"));
    button->setAutoRaise(true);
    button->setCursor(Qt::WhatsThisCursor);
    button->setToolTip(tooltip);
    button->setAccessibleName(QStringLiteral("Setting information"));
    button->setFixedWidth(24);
    return button;
}

void addSettingRow(
    QGridLayout *layout,
    int &row,
    const QString &labelText,
    QWidget *control,
    const QString &description,
    const QString &tooltip = QString()
)
{
    auto *label = new QLabel(labelText, layout->parentWidget());
    label->setMinimumWidth(LabelColumnWidth);
    label->setAlignment(Qt::AlignRight | Qt::AlignTop);
    label->setToolTip(tooltip);

    if (!tooltip.trimmed().isEmpty()) {
        control->setToolTip(tooltip);
    }

    layout->addWidget(label, row, 0, Qt::AlignRight | Qt::AlignTop);
    layout->addWidget(control, row, 1);

    if (!tooltip.trimmed().isEmpty()) {
        layout->addWidget(makeInfoButton(tooltip, layout->parentWidget()), row, 2, Qt::AlignTop);
    } else {
        layout->addItem(new QSpacerItem(24, 1, QSizePolicy::Fixed, QSizePolicy::Minimum), row, 2);
    }

    ++row;

    if (!description.trimmed().isEmpty()) {
        auto *desc = makeDescription(description, layout->parentWidget());
        layout->addWidget(desc, row, 1, 1, 2);
        ++row;
    }

    layout->setRowMinimumHeight(row, 8);
    ++row;
}

QWidget *wrapPage(QWidget *content)
{
    auto *scroll = new QScrollArea(content->parentWidget());
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setWidget(content);
    return scroll;
}

QFrame *makeWarningBox(const QString &text, QWidget *parent)
{
    auto *frame = new QFrame(parent);
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setObjectName(QStringLiteral("securityWarningBox"));
    frame->setStyleSheet(QStringLiteral(
        "QFrame#securityWarningBox {"
        "  border: 1px solid #9a6a00;"
        "  border-radius: 6px;"
        "  background: rgba(179, 107, 0, 28);"
        "}"
    ));

    auto *layout = new QVBoxLayout(frame);
    layout->setContentsMargins(12, 10, 12, 10);

    auto *label = new QLabel(text, frame);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setStyleSheet(QStringLiteral("color: #c98200;"));
    layout->addWidget(label);

    return frame;
}
}

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
    setMinimumSize(860, 560);
    resize(980, 640);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(14, 14, 14, 14);
    mainLayout->setSpacing(10);

    auto *titleLabel = new QLabel(QStringLiteral("DD-SSH settings"), this);
    titleLabel->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 16px;"));
    mainLayout->addWidget(titleLabel);

    auto *introLabel = makeDescription(
        QStringLiteral("Settings are stored in dd-ssh.json under the top-level settings block. "
                       "Theme changes apply after saving; terminal font changes apply to newly opened terminal tabs."),
        this
    );
    mainLayout->addWidget(introLabel);

    auto *bodyLayout = new QHBoxLayout();
    bodyLayout->setSpacing(12);
    mainLayout->addLayout(bodyLayout, 1);

    m_categoryList = new QListWidget(this);
    m_categoryList->setFixedWidth(180);
    m_categoryList->setSpacing(2);
    m_categoryList->setUniformItemSizes(true);
    m_categoryList->setAlternatingRowColors(false);
    bodyLayout->addWidget(m_categoryList);

    m_pages = new QStackedWidget(this);
    bodyLayout->addWidget(m_pages, 1);

    auto addPage = [this](const QString &name, QWidget *page) {
        m_categoryList->addItem(name);
        m_pages->addWidget(wrapPage(page));
    };

    // General page
    auto *generalPage = new QWidget(this);
    auto *generalLayout = new QVBoxLayout(generalPage);
    generalLayout->setContentsMargins(0, 0, 10, 0);
    generalLayout->setSpacing(14);
    generalLayout->addWidget(makePageTitle(QStringLiteral("General"), generalPage));

    auto *generalGridWidget = new QWidget(generalPage);
    auto *generalGrid = new QGridLayout(generalGridWidget);
    generalGrid->setContentsMargins(0, 0, 0, 0);
    generalGrid->setHorizontalSpacing(10);
    generalGrid->setVerticalSpacing(4);
    generalGrid->setColumnStretch(1, 1);
    int generalRow = 0;

    auto *configLabel = new QLabel(QStringLiteral("Config file:"), generalGridWidget);
    configLabel->setMinimumWidth(LabelColumnWidth);
    configLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);
    const QString configPathTooltip = QStringLiteral("Copy the path or open the folder containing the active DD-SSH configuration file.");
    configLabel->setToolTip(configPathTooltip);

    m_configPathEdit = new QLineEdit(generalGridWidget);
    m_configPathEdit->setText(m_configFilePath);
    m_configPathEdit->setReadOnly(true);
    m_configPathEdit->setMinimumWidth(0);
    m_configPathEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_configPathEdit->setToolTip(m_configFilePath);

    auto *configButtonRow = new QWidget(generalGridWidget);
    auto *configButtonLayout = new QHBoxLayout(configButtonRow);
    configButtonLayout->setContentsMargins(0, 0, 0, 0);
    configButtonLayout->setSpacing(8);

    auto *copyConfigPathButton = new QPushButton(QStringLiteral("Copy path"), configButtonRow);
    auto *openConfigFolderButton = new QPushButton(QStringLiteral("Open folder"), configButtonRow);
    copyConfigPathButton->setToolTip(configPathTooltip);
    openConfigFolderButton->setToolTip(configPathTooltip);

    configButtonLayout->addWidget(copyConfigPathButton);
    configButtonLayout->addWidget(openConfigFolderButton);
    configButtonLayout->addWidget(makeInfoButton(configPathTooltip, configButtonRow));
    configButtonLayout->addStretch(1);

    generalGrid->addWidget(configLabel, generalRow, 0, Qt::AlignRight | Qt::AlignTop);
    generalGrid->addWidget(m_configPathEdit, generalRow, 1, 1, 2);
    ++generalRow;

    generalGrid->addWidget(configButtonRow, generalRow, 1, 1, 2);
    ++generalRow;

    auto *configDesc = makeDescription(QStringLiteral("Read-only path to the active dd-ssh.json file."), generalGridWidget);
    generalGrid->addWidget(configDesc, generalRow, 1, 1, 2);
    ++generalRow;

    generalGrid->setRowMinimumHeight(generalRow, 8);
    ++generalRow;

    m_doubleClickLabel = new QLabel(QStringLiteral("Open terminal"), generalGridWidget);
    addSettingRow(
        generalGrid,
        generalRow,
        QStringLiteral("Double-click session:"),
        m_doubleClickLabel,
        QStringLiteral("Double-clicking a saved session opens a terminal tab."),
        QStringLiteral("The double-click action is currently fixed to Open terminal. File Manager remains available from the session context menu.")
    );

    m_showQuickToolbarCheck = new QCheckBox(QStringLiteral("Show quick action toolbar"), generalGridWidget);
    m_showQuickToolbarCheck->setChecked(settings.showQuickToolbar);
    addSettingRow(
        generalGrid,
        generalRow,
        QStringLiteral("Quick toolbar:"),
        m_showQuickToolbarCheck,
        QStringLiteral("When hidden, all actions remain available from the File, Session, Tools, and Help menus."),
        QStringLiteral("Shows optional top-level shortcuts such as New Session, Connect, Multi-Exec, and Settings.")
    );

    generalLayout->addWidget(generalGridWidget);
    generalLayout->addStretch(1);
    addPage(QStringLiteral("General"), generalPage);

    // Diagnostics page
    auto *diagnosticsPage = new QWidget(this);
    auto *diagnosticsLayout = new QVBoxLayout(diagnosticsPage);
    diagnosticsLayout->setContentsMargins(0, 0, 10, 0);
    diagnosticsLayout->setSpacing(14);
    diagnosticsLayout->addWidget(makePageTitle(QStringLiteral("Diagnostics"), diagnosticsPage));

    auto *diagnosticsGridWidget = new QWidget(diagnosticsPage);
    auto *diagnosticsGrid = new QGridLayout(diagnosticsGridWidget);
    diagnosticsGrid->setContentsMargins(0, 0, 0, 0);
    diagnosticsGrid->setHorizontalSpacing(10);
    diagnosticsGrid->setVerticalSpacing(4);
    diagnosticsGrid->setColumnStretch(1, 1);
    int diagnosticsRow = 0;

    m_diagnosticLoggingCheck = new QCheckBox(QStringLiteral("Enable diagnostic logging"), diagnosticsGridWidget);
    m_diagnosticLoggingCheck->setChecked(settings.diagnosticLoggingEnabled);
    addSettingRow(
        diagnosticsGrid,
        diagnosticsRow,
        QStringLiteral("Logging:"),
        m_diagnosticLoggingCheck,
        QStringLiteral("Off by default. Use Help → Open Log Folder to view logs when troubleshooting."),
        QStringLiteral("Writes diagnostic events such as app start, SSH/SFTP lifecycle, queue actions, transfer results, and errors. Secrets, terminal input/output, clipboard content, and file contents are not logged.")
    );

    diagnosticsLayout->addWidget(diagnosticsGridWidget);
    diagnosticsLayout->addStretch(1);
    addPage(QStringLiteral("Diagnostics"), diagnosticsPage);

    // Appearance page
    auto *appearancePage = new QWidget(this);
    auto *appearanceLayout = new QVBoxLayout(appearancePage);
    appearanceLayout->setContentsMargins(0, 0, 10, 0);
    appearanceLayout->setSpacing(14);
    appearanceLayout->addWidget(makePageTitle(QStringLiteral("Appearance"), appearancePage));

    auto *appearanceGridWidget = new QWidget(appearancePage);
    auto *appearanceGrid = new QGridLayout(appearanceGridWidget);
    appearanceGrid->setContentsMargins(0, 0, 0, 0);
    appearanceGrid->setHorizontalSpacing(10);
    appearanceGrid->setVerticalSpacing(4);
    appearanceGrid->setColumnStretch(1, 1);
    int appearanceRow = 0;

    m_appThemeCombo = new QComboBox(appearanceGridWidget);
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

    addSettingRow(
        appearanceGrid,
        appearanceRow,
        QStringLiteral("App theme:"),
        m_appThemeCombo,
        QStringLiteral("Changes the Qt app chrome. The terminal renderer currently keeps its dark xterm.js theme."),
        QStringLiteral("System default follows the operating system. Light and Dark force the DD-SSH application UI only.")
    );

    appearanceLayout->addWidget(appearanceGridWidget);
    appearanceLayout->addStretch(1);
    addPage(QStringLiteral("Appearance"), appearancePage);

    // Terminal page
    auto *terminalPage = new QWidget(this);
    auto *terminalLayout = new QVBoxLayout(terminalPage);
    terminalLayout->setContentsMargins(0, 0, 10, 0);
    terminalLayout->setSpacing(14);
    terminalLayout->addWidget(makePageTitle(QStringLiteral("Terminal"), terminalPage));

    auto *terminalGridWidget = new QWidget(terminalPage);
    auto *terminalGrid = new QGridLayout(terminalGridWidget);
    terminalGrid->setContentsMargins(0, 0, 0, 0);
    terminalGrid->setHorizontalSpacing(10);
    terminalGrid->setVerticalSpacing(4);
    terminalGrid->setColumnStretch(1, 1);
    int terminalRow = 0;

    m_terminalFontFamilyEdit = new QLineEdit(terminalGridWidget);
    m_terminalFontFamilyEdit->setText(settings.terminalFontFamily.trimmed().isEmpty()
        ? QStringLiteral("monospace")
        : settings.terminalFontFamily.trimmed());
    m_terminalFontFamilyEdit->setPlaceholderText(QStringLiteral("monospace"));
    addSettingRow(
        terminalGrid,
        terminalRow,
        QStringLiteral("Font family:"),
        m_terminalFontFamilyEdit,
        QStringLiteral("Applies to newly opened xterm.js terminal tabs."),
        QStringLiteral("Use a monospace font installed on your system. Existing terminal tabs are not changed.")
    );

    m_terminalFontSizeSpin = new QSpinBox(terminalGridWidget);
    m_terminalFontSizeSpin->setRange(8, 36);
    m_terminalFontSizeSpin->setValue(settings.terminalFontSize);
    addSettingRow(
        terminalGrid,
        terminalRow,
        QStringLiteral("Font size:"),
        m_terminalFontSizeSpin,
        QStringLiteral("Applies to newly opened xterm.js terminal tabs."),
        QStringLiteral("Terminal font size in points for new tabs.")
    );

    terminalLayout->addWidget(terminalGridWidget);
    terminalLayout->addStretch(1);
    addPage(QStringLiteral("Terminal"), terminalPage);

    // Config safety page
    auto *configSafetyPage = new QWidget(this);
    auto *configSafetyLayout = new QVBoxLayout(configSafetyPage);
    configSafetyLayout->setContentsMargins(0, 0, 10, 0);
    configSafetyLayout->setSpacing(14);
    configSafetyLayout->addWidget(makePageTitle(QStringLiteral("Config safety"), configSafetyPage));

    auto *configSafetyGridWidget = new QWidget(configSafetyPage);
    auto *configSafetyGrid = new QGridLayout(configSafetyGridWidget);
    configSafetyGrid->setContentsMargins(0, 0, 0, 0);
    configSafetyGrid->setHorizontalSpacing(10);
    configSafetyGrid->setVerticalSpacing(4);
    configSafetyGrid->setColumnStretch(1, 1);
    int configSafetyRow = 0;

    m_configBackupsCheck = new QCheckBox(QStringLiteral("Enable config backups before save"), configSafetyGridWidget);
    m_configBackupsCheck->setChecked(settings.configBackupsEnabled);
    addSettingRow(
        configSafetyGrid,
        configSafetyRow,
        QStringLiteral("Backups:"),
        m_configBackupsCheck,
        QStringLiteral("Creates a rotating backup of dd-ssh.json before saving config changes."),
        QStringLiteral("Recommended while DD-SSH is in alpha. Backups help recover from accidental config edits or failed imports.")
    );

    m_maxBackupsSpin = new QSpinBox(configSafetyGridWidget);
    m_maxBackupsSpin->setRange(1, 50);
    m_maxBackupsSpin->setValue(settings.maxConfigBackups);
    addSettingRow(
        configSafetyGrid,
        configSafetyRow,
        QStringLiteral("Keep last:"),
        m_maxBackupsSpin,
        QStringLiteral("Maximum number of rotating config backups to keep."),
        QStringLiteral("Older backup files are removed when the configured limit is exceeded.")
    );

    configSafetyLayout->addWidget(configSafetyGridWidget);
    configSafetyLayout->addStretch(1);
    addPage(QStringLiteral("Config safety"), configSafetyPage);

    // Security note page
    auto *securityPage = new QWidget(this);
    auto *securityLayout = new QVBoxLayout(securityPage);
    securityLayout->setContentsMargins(0, 0, 10, 0);
    securityLayout->setSpacing(14);
    securityLayout->addWidget(makePageTitle(QStringLiteral("Security note"), securityPage));
    securityLayout->addWidget(makeWarningBox(
        QStringLiteral("DD-SSH currently uses secrets.mode = plain-v1.\n\n"
                       "Saved passwords and private keys are portable but stored in plaintext inside dd-ssh.json. "
                       "Use this alpha build only on trusted machines and keep exported configs protected."),
        securityPage
    ));
    securityLayout->addStretch(1);
    addPage(QStringLiteral("Security note"), securityPage);

    if (m_categoryList->count() > 0) {
        m_categoryList->setCurrentRow(0);
        m_pages->setCurrentIndex(0);
    }

    connect(m_categoryList, &QListWidget::currentRowChanged, m_pages, &QStackedWidget::setCurrentIndex);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this
    );
    mainLayout->addWidget(buttons);

    connect(copyConfigPathButton, &QPushButton::clicked, this, [this]() {
        if (QApplication::clipboard() != nullptr) {
            QApplication::clipboard()->setText(m_configFilePath);
            QMessageBox::information(this, QStringLiteral("Config path copied"), QStringLiteral("Config file path copied to clipboard."));
        }
    });

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
            m_categoryList->setCurrentRow(3);
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
