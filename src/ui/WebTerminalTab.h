#pragma once

#include "core/SessionProfile.h"
#include "core/ConfigManager.h"
#include "ssh/SshSession.h"

#include <QWidget>

class QShowEvent;

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
        const SshHostKeyExpectation &hostKeyExpectation = SshHostKeyExpectation(),
        QWidget *parent = nullptr
    );

    ~WebTerminalTab() override;

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

protected:
    void showEvent(QShowEvent *event) override;

private:
    QString terminalHtml() const;
    QString targetLabel() const;
    void startShell();
    void sendToWorker(const QString &input);
    void copySelection();
    void pasteClipboard();
    void clearTerminal();
    void resetTerminal();
    void focusTerminal();
    void disconnectShell();
    void reconnectShell();
    void setTerminalInputEnabled(bool enabled);
    void setConnectionUiState(const QString &state, bool remoteInputEnabled, bool reconnectAvailable);
    void updateTerminalConnectionState(const QString &state);
    void handleWorkerFinished();
    void requestPtyResize(int columns, int rows);

    SessionProfile m_session;
    QString m_secretValue;
    SshHostKeyExpectation m_hostKeyExpectation;
    AppSettings m_appSettings;

    QLabel *m_statusLabel = nullptr;
    QWebEngineView *m_view = nullptr;
    QWebChannel *m_channel = nullptr;
    TerminalBridge *m_bridge = nullptr;
    QPushButton *m_interruptButton = nullptr;
    QPushButton *m_pasteButton = nullptr;
    QPushButton *m_clearButton = nullptr;
    QPushButton *m_focusButton = nullptr;
    QPushButton *m_resetButton = nullptr;
    QPushButton *m_reconnectButton = nullptr;
    QPushButton *m_disconnectButton = nullptr;

    QThread *m_thread = nullptr;
    SshShellWorker *m_worker = nullptr;
    int m_lastTerminalColumns = 0;
    int m_lastTerminalRows = 0;
    qint64 m_receivedBytesTotal = 0;
    qint64 m_sentBytesTotal = 0;
    bool m_shellStarted = false;
    bool m_shellActive = false;
    bool m_disconnectRequested = false;
};
