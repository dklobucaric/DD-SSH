#pragma once

#include "core/SessionProfile.h"
#include "ssh/SshSession.h"

#include <QString>
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

    bool hasActiveShell() const;
    QString displayName() const;
    void requestDisconnect();

signals:
    void tabTitleChanged(const QString &title);
    void lifecycleStatusChanged(const QString &status);

private slots:
    void sendCurrentInput();
    void sendInterrupt();
    void clearOutput();
    void appendOutput(const QString &output);
    void updateState(const QString &state);
    void showWorkerError(const QString &error);
    void handleWorkerFinished();
    void disconnectShell();

private:
    QString cleanTerminalOutput(const QString &output) const;

    SessionProfile m_session;
    QString m_secretValue;

    QLabel *m_statusLabel = nullptr;
    QPlainTextEdit *m_output = nullptr;
    QLineEdit *m_input = nullptr;
    QPushButton *m_sendButton = nullptr;
    QPushButton *m_interruptButton = nullptr;
    QPushButton *m_clearButton = nullptr;
    QPushButton *m_disconnectButton = nullptr;

    QThread *m_thread = nullptr;
    SshShellWorker *m_worker = nullptr;
    bool m_shellActive = false;
    bool m_disconnectRequested = false;
};
