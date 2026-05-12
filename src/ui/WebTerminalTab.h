#pragma once

#include "core/SessionProfile.h"

#include <QWidget>

class QLabel;
class QPushButton;
class QThread;
class QWebChannel;
class QWebEngineView;
class SshShellWorker;
class TerminalBridge;

class WebTerminalTab : public QWidget
{
    Q_OBJECT

public:
    WebTerminalTab(
        const SessionProfile &session,
        const QString &secretValue,
        QWidget *parent = nullptr
    );

    ~WebTerminalTab() override;

private:
    QString terminalHtml() const;
    void startShell();
    void sendToWorker(const QString &input);
    void sendInterrupt();
    void pasteClipboard();
    void clearTerminal();
    void disconnectShell();
    void handleWorkerFinished();

    SessionProfile m_session;
    QString m_secretValue;

    QLabel *m_statusLabel = nullptr;
    QWebEngineView *m_view = nullptr;
    QWebChannel *m_channel = nullptr;
    TerminalBridge *m_bridge = nullptr;
    QPushButton *m_interruptButton = nullptr;
    QPushButton *m_pasteButton = nullptr;
    QPushButton *m_clearButton = nullptr;
    QPushButton *m_disconnectButton = nullptr;

    QThread *m_thread = nullptr;
    SshShellWorker *m_worker = nullptr;
    bool m_shellStarted = false;
};
