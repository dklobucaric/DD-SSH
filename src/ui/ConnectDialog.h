#pragma once

#include <QDialog>
#include <QString>

class QComboBox;
class QLineEdit;
class QSpinBox;

class ConnectDialog : public QDialog
{
public:
    enum class AuthType
    {
        Password,
        PrivateKey
    };

    explicit ConnectDialog(QWidget *parent = nullptr);

    QString host() const;
    int port() const;
    QString username() const;
    AuthType authType() const;
    QString password() const;
    QString keyPath() const;

private:
    void updateAuthFields();

    QLineEdit *m_hostEdit = nullptr;
    QSpinBox *m_portSpin = nullptr;
    QLineEdit *m_usernameEdit = nullptr;
    QComboBox *m_authTypeCombo = nullptr;
    QLineEdit *m_passwordEdit = nullptr;
    QLineEdit *m_keyPathEdit = nullptr;
};
