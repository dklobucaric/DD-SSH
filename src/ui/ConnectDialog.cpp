#include "ConnectDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
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
    resize(460, 260);

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

    mainLayout->addLayout(form);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this
    );

    mainLayout->addWidget(buttons);

    connect(m_authTypeCombo, &QComboBox::currentTextChanged, this, [this]() {
        updateAuthFields();
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

        accept();
    });

    updateAuthFields();
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

void ConnectDialog::updateAuthFields()
{
    const bool usePassword = authType() == AuthType::Password;

    m_passwordEdit->setEnabled(usePassword);
    m_keyPathEdit->setEnabled(!usePassword);
}
