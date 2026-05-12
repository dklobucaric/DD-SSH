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
    setWindowTitle("Connect to SSH server");
    setModal(true);
    resize(520, 390);

    auto *mainLayout = new QVBoxLayout(this);

    auto *title = new QLabel("Create a manual SSH connection", this);
    title->setStyleSheet("font-weight: bold;");
    mainLayout->addWidget(title);

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
    m_saveConnectionCheck->setChecked(true);
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

        if (authType() == AuthType::Password && password().isEmpty()) {
            QMessageBox::warning(this, "Missing password", "Please enter password.");
            m_passwordEdit->setFocus();
            return;
        }

        if (authType() == AuthType::PrivateKey && keyPath().trimmed().isEmpty()) {
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
    m_plainTextWarningLabel->setVisible(save);
}

void ConnectDialog::refreshDefaultSessionName()
{
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
