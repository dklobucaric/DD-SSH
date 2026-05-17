#include "ConnectDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

ConnectDialog::ConnectDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Manual SSH connect / auth test");
    setModal(true);
    resize(560, 460);

    auto *mainLayout = new QVBoxLayout(this);

    m_titleLabel = new QLabel("Manual SSH connection/auth test", this);
    m_titleLabel->setStyleSheet("font-weight: bold;");
    mainLayout->addWidget(m_titleLabel);

    auto *form = new QFormLayout();

    m_hostEdit = new QLineEdit(this);
    m_hostEdit->setPlaceholderText("example.com or 192.168.1.10");
    form->addRow("Host:", m_hostEdit);

    m_portSpin = new QSpinBox(this);
    m_portSpin->setRange(1, 65535);
    m_portSpin->setValue(22);
    form->addRow("Port:", m_portSpin);

    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("root, user, admin...");
    form->addRow("Username:", m_usernameEdit);

    m_authTypeCombo = new QComboBox(this);
    m_authTypeCombo->addItem("Password", "password");
    m_authTypeCombo->addItem("Private key", "key");
    form->addRow("Auth type:", m_authTypeCombo);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Password");
    form->addRow("Password:", m_passwordEdit);

    auto *keyPathRow = new QWidget(this);
    auto *keyPathLayout = new QHBoxLayout(keyPathRow);
    keyPathLayout->setContentsMargins(0, 0, 0, 0);

    m_keyPathEdit = new QLineEdit(keyPathRow);
    m_keyPathEdit->setPlaceholderText("~/.ssh/id_ed25519");

    auto *browseButton = new QPushButton("Browse...", keyPathRow);

    keyPathLayout->addWidget(m_keyPathEdit);
    keyPathLayout->addWidget(browseButton);

    form->addRow("Private key:", keyPathRow);

    m_saveConnectionCheck = new QCheckBox("Save this connection", this);
    m_saveConnectionCheck->setChecked(false);
    form->addRow("", m_saveConnectionCheck);

    m_sessionNameEdit = new QLineEdit(this);
    m_sessionNameEdit->setPlaceholderText("Example: Nextcloud Backend");
    form->addRow("Session name:", m_sessionNameEdit);

    m_groupEdit = new QLineEdit(this);
    m_groupEdit->setPlaceholderText("Example: DD-Lab");
    form->addRow("Group:", m_groupEdit);

    mainLayout->addLayout(form);

    m_plainTextWarningLabel = new QLabel(
        "Saved passwords/private keys are stored in plaintext inside dd-ssh.json in this early portable mode.",
        this
    );
    m_plainTextWarningLabel->setWordWrap(true);
    m_plainTextWarningLabel->setStyleSheet("color: #b36b00;");
    mainLayout->addWidget(m_plainTextWarningLabel);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this
    );

    mainLayout->addWidget(buttons);

    connect(m_authTypeCombo, &QComboBox::currentTextChanged, this, [this]() {
        updateAuthFields();
    });

    connect(m_saveConnectionCheck, &QCheckBox::toggled, this, [this]() {
        updateSaveFields();
    });

    connect(m_hostEdit, &QLineEdit::textChanged, this, [this]() {
        refreshDefaultSessionName();
    });

    connect(m_usernameEdit, &QLineEdit::textChanged, this, [this]() {
        refreshDefaultSessionName();
    });

    connect(m_portSpin, &QSpinBox::valueChanged, this, [this]() {
        refreshDefaultSessionName();
    });

    connect(browseButton, &QPushButton::clicked, this, [this]() {
        const QString fileName = QFileDialog::getOpenFileName(
            this,
            "Select private key",
            QDir::homePath() + "/.ssh"
        );

        if (!fileName.isEmpty()) {
            m_keyPathEdit->setText(fileName);
        }
    });

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        if (host().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Missing host", "Please enter SSH host.");
            m_hostEdit->setFocus();
            return;
        }

        if (username().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Missing username", "Please enter username.");
            m_usernameEdit->setFocus();
            return;
        }

        if (authType() == AuthType::Password && password().isEmpty() && !m_editMode) {
            QMessageBox::warning(this, "Missing password", "Please enter password.");
            m_passwordEdit->setFocus();
            return;
        }

        if (authType() == AuthType::PrivateKey && keyPath().trimmed().isEmpty() && !m_editMode) {
            QMessageBox::warning(this, "Missing private key", "Please select private key path.");
            m_keyPathEdit->setFocus();
            return;
        }

        if (saveConnection() && sessionName().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Missing session name", "Please enter session name or disable saving.");
            m_sessionNameEdit->setFocus();
            return;
        }

        accept();
    });

    setDialogMode(DialogMode::ManualConnect);
    updateAuthFields();
    updateSaveFields();
    refreshDefaultSessionName();
}

QString ConnectDialog::host() const
{
    return m_hostEdit->text().trimmed();
}

int ConnectDialog::port() const
{
    return m_portSpin->value();
}

QString ConnectDialog::username() const
{
    return m_usernameEdit->text().trimmed();
}

ConnectDialog::AuthType ConnectDialog::authType() const
{
    const QString value = m_authTypeCombo->currentData().toString();

    if (value == "key") {
        return AuthType::PrivateKey;
    }

    return AuthType::Password;
}

QString ConnectDialog::password() const
{
    return m_passwordEdit->text();
}

QString ConnectDialog::keyPath() const
{
    return m_keyPathEdit->text().trimmed();
}

bool ConnectDialog::saveConnection() const
{
    return m_saveConnectionCheck->isChecked();
}

QString ConnectDialog::sessionName() const
{
    return m_sessionNameEdit->text().trimmed();
}

QString ConnectDialog::groupName() const
{
    return m_groupEdit->text().trimmed();
}

void ConnectDialog::setDialogMode(DialogMode mode)
{
    m_mode = mode;
    m_editMode = mode == DialogMode::EditSession;

    if (m_mode == DialogMode::EditSession) {
        setWindowTitle("Edit saved SSH session");
        m_titleLabel->setText("Edit saved SSH session");
        m_saveConnectionCheck->setText("Save changes to this saved session");
        m_saveConnectionCheck->setChecked(true);
        m_saveConnectionCheck->setEnabled(false);
        m_passwordEdit->setPlaceholderText("Leave empty to keep the saved password");
        m_keyPathEdit->setPlaceholderText("Leave empty to keep the saved private key, or browse a new key file");
        m_plainTextWarningLabel->setText(
            "Editing a saved session keeps the existing plaintext secret if password/private key is left empty. "
            "Entering a new password or key replaces the saved secret in dd-ssh.json."
        );
    } else if (m_mode == DialogMode::NewSession) {
        setWindowTitle("New saved SSH session");
        m_titleLabel->setText("Create a new saved SSH session");
        m_saveConnectionCheck->setText("Save this connection to dd-ssh.json");
        m_saveConnectionCheck->setChecked(true);
        m_saveConnectionCheck->setEnabled(false);
        m_passwordEdit->setPlaceholderText("Password to store in plaintext plain-v1 secrets");
        m_keyPathEdit->setPlaceholderText("~/.ssh/id_ed25519 — file content will be copied into dd-ssh.json");
        m_plainTextWarningLabel->setText(
            "New Session creates a saved session after a successful auth test. "
            "Passwords/private keys are stored in plaintext inside dd-ssh.json in this early portable mode."
        );
    } else {
        setWindowTitle("Manual SSH connect / auth test");
        m_titleLabel->setText("Manual SSH connection/auth test");
        m_saveConnectionCheck->setText("Save this connection after successful auth");
        m_saveConnectionCheck->setChecked(false);
        m_saveConnectionCheck->setEnabled(true);
        m_passwordEdit->setPlaceholderText("Password");
        m_keyPathEdit->setPlaceholderText("~/.ssh/id_ed25519");
        m_plainTextWarningLabel->setText(
            "Saving is optional for Manual Connect. If enabled, passwords/private keys are stored in plaintext inside dd-ssh.json."
        );
    }

    updateSaveFields();
    updateAuthFields();
}

ConnectDialog::DialogMode ConnectDialog::dialogMode() const
{
    return m_mode;
}

void ConnectDialog::setEditMode(bool editMode)
{
    setDialogMode(editMode ? DialogMode::EditSession : DialogMode::ManualConnect);
}

bool ConnectDialog::isEditMode() const
{
    return m_editMode;
}

void ConnectDialog::setConnectionFields(
    const QString &host,
    int port,
    const QString &username,
    AuthType authType
)
{
    m_hostEdit->setText(host.trimmed());
    m_portSpin->setValue(port > 0 ? port : 22);
    m_usernameEdit->setText(username.trimmed());

    const int authIndex = m_authTypeCombo->findData(
        authType == AuthType::PrivateKey
            ? QStringLiteral("key")
            : QStringLiteral("password")
    );

    if (authIndex >= 0) {
        m_authTypeCombo->setCurrentIndex(authIndex);
    }

    updateAuthFields();
}

void ConnectDialog::setSessionFields(
    const QString &sessionName,
    const QString &groupName
)
{
    m_sessionNameEdit->setText(sessionName.trimmed());
    m_groupEdit->setText(groupName.trimmed());
}

void ConnectDialog::updateAuthFields()
{
    const bool usePassword = authType() == AuthType::Password;

    m_passwordEdit->setEnabled(usePassword);
    m_keyPathEdit->setEnabled(!usePassword);
}

void ConnectDialog::updateSaveFields()
{
    const bool save = saveConnection();

    m_sessionNameEdit->setEnabled(save);
    m_groupEdit->setEnabled(save);
    m_plainTextWarningLabel->setVisible(save || m_editMode);
}

void ConnectDialog::refreshDefaultSessionName()
{
    if (m_editMode) {
        return;
    }

    if (m_sessionNameEdit == nullptr || !m_sessionNameEdit->text().trimmed().isEmpty()) {
        return;
    }

    if (host().isEmpty() || username().isEmpty()) {
        return;
    }

    m_sessionNameEdit->setText(
        username()
        + QStringLiteral("@")
        + host()
        + QStringLiteral(":")
        + QString::number(port())
    );
}
