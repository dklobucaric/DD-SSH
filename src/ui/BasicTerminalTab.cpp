#include "BasicTerminalTab.h"
#include "ssh/SshShellWorker.h"

#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollBar>
#include <QTextCursor>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>

BasicTerminalTab::BasicTerminalTab(
    const SessionProfile &session,
    const QString &secretValue,
    const SshHostKeyExpectation &hostKeyExpectation,
    QWidget *parent
)
    : QWidget(parent)
    , m_session(session)
    , m_secretValue(secretValue)
    , m_hostKeyExpectation(hostKeyExpectation)
{
    auto *layout = new QVBoxLayout(this);

    const QString target =
        m_session.username
        + QStringLiteral("@")
        + m_session.host
        + QStringLiteral(":")
        + QString::number(m_session.port);

    m_statusLabel = new QLabel(QStringLiteral("Starting basic SSH shell for ") + target, this);
    layout->addWidget(m_statusLabel);

    m_output = new QPlainTextEdit(this);
    m_output->setReadOnly(true);
    m_output->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_output->setLineWrapMode(QPlainTextEdit::NoWrap);
    layout->addWidget(m_output, 1);

    auto *inputLayout = new QHBoxLayout();

    m_input = new QLineEdit(this);
    m_input->setPlaceholderText(QStringLiteral("Temporary input: type a command and press Enter. Real in-terminal typing comes with xterm.js."));
    inputLayout->addWidget(m_input, 1);

    m_sendButton = new QPushButton(QStringLiteral("Send"), this);
    inputLayout->addWidget(m_sendButton);

    m_interruptButton = new QPushButton(QStringLiteral("Ctrl+C"), this);
    m_interruptButton->setToolTip(QStringLiteral("Send Ctrl+C to the remote shell."));
    inputLayout->addWidget(m_interruptButton);

    m_clearButton = new QPushButton(QStringLiteral("Clear"), this);
    m_clearButton->setToolTip(QStringLiteral("Clear local output view only. This does not run the remote clear command."));
    inputLayout->addWidget(m_clearButton);

    m_disconnectButton = new QPushButton(QStringLiteral("Disconnect"), this);
    inputLayout->addWidget(m_disconnectButton);

    layout->addLayout(inputLayout);

    connect(m_input, &QLineEdit::returnPressed, this, &BasicTerminalTab::sendCurrentInput);
    connect(m_sendButton, &QPushButton::clicked, this, &BasicTerminalTab::sendCurrentInput);
    connect(m_interruptButton, &QPushButton::clicked, this, &BasicTerminalTab::sendInterrupt);
    connect(m_clearButton, &QPushButton::clicked, this, &BasicTerminalTab::clearOutput);
    connect(m_disconnectButton, &QPushButton::clicked, this, &BasicTerminalTab::disconnectShell);

    const SshAuthMethod authMethod = m_session.authType == SessionProfile::AuthType::PrivateKey
        ? SshAuthMethod::PrivateKey
        : SshAuthMethod::Password;

    m_thread = new QThread(this);
    m_worker = new SshShellWorker(
        m_session.host,
        m_session.port,
        m_session.username,
        authMethod,
        m_secretValue,
        m_hostKeyExpectation,
        displayName()
    );

    m_receivedBytesTotal = 0;
    m_sentBytesTotal = 0;
    emit trafficCountersChanged();

    m_shellActive = true;
    m_disconnectRequested = false;
    emit tabTitleChanged(displayName() + QStringLiteral(" ●"));
    emit lifecycleStatusChanged(QStringLiteral("Connecting ") + displayName());

    m_worker->moveToThread(m_thread);

    connect(m_thread, &QThread::started, m_worker, &SshShellWorker::start);
    connect(m_worker, &SshShellWorker::outputReceived, this, &BasicTerminalTab::appendOutput);
    connect(m_worker, &SshShellWorker::trafficUpdated, this, [this](qint64 receivedBytes, qint64 sentBytes) {
        m_receivedBytesTotal = receivedBytes;
        m_sentBytesTotal = sentBytes;
        emit trafficCountersChanged();
    });
    connect(m_worker, &SshShellWorker::stateChanged, this, &BasicTerminalTab::updateState);
    connect(m_worker, &SshShellWorker::errorOccurred, this, &BasicTerminalTab::showWorkerError);
    connect(m_worker, &SshShellWorker::finished, this, &BasicTerminalTab::handleWorkerFinished);
    connect(m_worker, &SshShellWorker::finished, m_thread, &QThread::quit);
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_thread, &QThread::finished, this, [this]() {
        m_worker = nullptr;
    });

    m_output->appendPlainText(QStringLiteral("DD-SSH basic shell channel test\n"));
    m_output->appendPlainText(QStringLiteral("Session: ") + m_session.name);
    m_output->appendPlainText(QStringLiteral("Target: ") + target);
    m_output->appendPlainText(QStringLiteral("\nThis is an early channel milestone."));
    m_output->appendPlainText(QStringLiteral("It is not xterm.js yet, so full-screen programs like htop/nano/vim are not expected to behave correctly."));
    m_output->appendPlainText(QStringLiteral("Common ANSI escape sequences are hidden in this temporary view to keep the output readable.\n"));

    m_thread->start();

    QTimer::singleShot(0, m_input, [this]() {
        if (m_input != nullptr) {
            m_input->setFocus();
        }
    });
}

BasicTerminalTab::~BasicTerminalTab()
{
    disconnectShell();

    if (m_thread != nullptr) {
        m_thread->quit();
        m_thread->wait(3000);
    }
}

bool BasicTerminalTab::hasActiveShell() const
{
    return m_shellActive && m_worker != nullptr;
}

QString BasicTerminalTab::displayName() const
{
    if (!m_session.name.trimmed().isEmpty()) {
        return m_session.name.trimmed();
    }

    return m_session.username
        + QStringLiteral("@")
        + m_session.host
        + QStringLiteral(":")
        + QString::number(m_session.port);
}

QString BasicTerminalTab::trafficSessionName() const
{
    return displayName();
}

qint64 BasicTerminalTab::receivedBytesTotal() const
{
    return m_receivedBytesTotal;
}

qint64 BasicTerminalTab::sentBytesTotal() const
{
    return m_sentBytesTotal;
}

void BasicTerminalTab::requestDisconnect()
{
    disconnectShell();
}

QString BasicTerminalTab::cleanTerminalOutput(const QString &output) const
{
    QString cleaned = output;

    // OSC sequences, for example: ESC ] 0 ; title BEL
    cleaned.remove(QRegularExpression(QStringLiteral("\x1B\\][^\x07]*(?:\x07|\x1B\\\\)")));

    // CSI sequences, for example colors, cursor moves, bracketed paste on/off.
    cleaned.remove(QRegularExpression(QStringLiteral("\x1B\\[[0-?]*[ -/]*[@-~]")));

    // Simple one-character ESC sequences.
    cleaned.remove(QRegularExpression(QStringLiteral("\x1B[@-Z\\\\-_]")));

    return cleaned;
}

void BasicTerminalTab::sendCurrentInput()
{
    if (!m_shellActive || m_worker == nullptr || m_input == nullptr) {
        return;
    }

    const QString command = m_input->text();

    if (command.isEmpty()) {
        m_worker->sendInput(QStringLiteral("\n"));
        m_input->setFocus();
        return;
    }

    m_worker->sendInput(command + QStringLiteral("\n"));
    m_input->clear();
    m_input->setFocus();
}

void BasicTerminalTab::sendInterrupt()
{
    if (!m_shellActive || m_worker == nullptr) {
        return;
    }

    m_worker->sendInput(QString(QChar(0x03)));
    appendOutput(QStringLiteral("\n[DD-SSH] Sent Ctrl+C to remote shell.\n"));

    if (m_input != nullptr) {
        m_input->setFocus();
    }
}

void BasicTerminalTab::clearOutput()
{
    if (m_output != nullptr) {
        m_output->clear();
    }

    if (m_input != nullptr) {
        m_input->setFocus();
    }
}

void BasicTerminalTab::appendOutput(const QString &output)
{
    if (m_output == nullptr) {
        return;
    }

    const QString displayOutput = cleanTerminalOutput(output);

    if (displayOutput.isEmpty()) {
        return;
    }

    m_output->moveCursor(QTextCursor::End);
    m_output->insertPlainText(displayOutput);
    m_output->moveCursor(QTextCursor::End);

    if (m_output->verticalScrollBar() != nullptr) {
        m_output->verticalScrollBar()->setValue(m_output->verticalScrollBar()->maximum());
    }
}

void BasicTerminalTab::updateState(const QString &state)
{
    if (m_statusLabel != nullptr) {
        m_statusLabel->setText(state);
    }

    if (state.contains(QStringLiteral("Connected"), Qt::CaseInsensitive)) {
        emit tabTitleChanged(displayName() + QStringLiteral(" ●"));
    }

    emit lifecycleStatusChanged(state);
    appendOutput(QStringLiteral("\n[DD-SSH] ") + state + QStringLiteral("\n"));
}

void BasicTerminalTab::showWorkerError(const QString &error)
{
    emit lifecycleStatusChanged(QStringLiteral("Shell error: ") + error);
    appendOutput(QStringLiteral("\n[DD-SSH ERROR] ") + error + QStringLiteral("\n"));

    if (m_statusLabel != nullptr) {
        m_statusLabel->setText(QStringLiteral("Error: ") + error);
    }
}

void BasicTerminalTab::handleWorkerFinished()
{
    m_shellActive = false;
    emit tabTitleChanged(displayName() + QStringLiteral(" ×"));
    emit lifecycleStatusChanged(QStringLiteral("Disconnected: ") + displayName());

    if (m_input != nullptr) {
        m_input->setEnabled(false);
    }

    if (m_sendButton != nullptr) {
        m_sendButton->setEnabled(false);
    }

    if (m_interruptButton != nullptr) {
        m_interruptButton->setEnabled(false);
    }

    if (m_disconnectButton != nullptr) {
        m_disconnectButton->setEnabled(false);
    }

    appendOutput(QStringLiteral("\n[DD-SSH] Shell worker finished.\n"));
}

void BasicTerminalTab::disconnectShell()
{
    if (m_worker != nullptr) {
        if (!m_disconnectRequested) {
            m_disconnectRequested = true;
            appendOutput(QStringLiteral("\n[DD-SSH] Disconnect requested.\n"));

            if (m_statusLabel != nullptr) {
                m_statusLabel->setText(QStringLiteral("Disconnect requested..."));
            }

            emit lifecycleStatusChanged(QStringLiteral("Disconnect requested for ") + displayName());
        }

        m_worker->stop();
    }
}
