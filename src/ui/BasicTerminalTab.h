#pragma once

#include "core/SessionProfile.h"
#include "ssh/SshSession.h"

#include <QWidget>

class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QThread;
class SshShellWorker;

class BasicTerminalTab : public QWidget
{
    Q_OBJECT

public:
    BasicTerminalTab(
        const SessionProfile &session,
        const QString &secretValue,
        QWidget *parent = nullptr
    );

    ~BasicTerminalTab() override;

private slots:
    void sendCurrentInput();
    void appendOutput(const QString &output);
    void updateState(const QString &state);
    void showWorkerError(const QString &error);
    void handleWorkerFinished();
    void disconnectShell();

private:
    SessionProfile m_session;
    QString m_secretValue;

    QLabel *m_statusLabel = nullptr;
    QPlainTextEdit *m_output = nullptr;
    QLineEdit *m_input = nullptr;
    QPushButton *m_sendButton = nullptr;
    QPushButton *m_disconnectButton = nullptr;

    QThread *m_thread = nullptr;
    SshShellWorker *m_worker = nullptr;
};
