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
        const SshHostKeyExpectation &hostKeyExpectation = SshHostKeyExpectation(),
        QWidget *parent = nullptr
    );

    ~BasicTerminalTab() override;

    bool hasActiveShell() const;
    QString displayName() const;
    QString trafficSessionName() const;
    qint64 receivedBytesTotal() const;
    qint64 sentBytesTotal() const;
    void requestDisconnect();

signals:
    void tabTitleChanged(const QString &title);
    void lifecycleStatusChanged(const QString &status);
    void trafficCountersChanged();

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
    SshHostKeyExpectation m_hostKeyExpectation;

    QLabel *m_statusLabel = nullptr;
    QPlainTextEdit *m_output = nullptr;
    QLineEdit *m_input = nullptr;
    QPushButton *m_sendButton = nullptr;
    QPushButton *m_interruptButton = nullptr;
    QPushButton *m_clearButton = nullptr;
    QPushButton *m_disconnectButton = nullptr;

    QThread *m_thread = nullptr;
    SshShellWorker *m_worker = nullptr;
    qint64 m_receivedBytesTotal = 0;
    qint64 m_sentBytesTotal = 0;
    bool m_shellActive = false;
    bool m_disconnectRequested = false;
};
