#include "BasicTerminalTab.h"
#include "ssh/SshShellWorker.h"

#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QTextCursor>
#include <QThread>
#include <QVBoxLayout>

BasicTerminalTab::BasicTerminalTab(
    const SessionProfile &session,
    const QString &secretValue,
    QWidget *parent
)
    : QWidget(parent)
    , m_session(session)
    , m_secretValue(secretValue)
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
    m_input->setPlaceholderText(QStringLiteral("Type command and press Enter. This is a basic channel test, not the final terminal emulator."));
    inputLayout->addWidget(m_input, 1);

    m_sendButton = new QPushButton(QStringLiteral("Send"), this);
    inputLayout->addWidget(m_sendButton);

    m_disconnectButton = new QPushButton(QStringLiteral("Disconnect"), this);
    inputLayout->addWidget(m_disconnectButton);

    layout->addLayout(inputLayout);

    connect(m_input, &QLineEdit::returnPressed, this, &BasicTerminalTab::sendCurrentInput);
    connect(m_sendButton, &QPushButton::clicked, this, &BasicTerminalTab::sendCurrentInput);
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
        m_secretValue
    );

    m_worker->moveToThread(m_thread);

    connect(m_thread, &QThread::started, m_worker, &SshShellWorker::start);
    connect(m_worker, &SshShellWorker::outputReceived, this, &BasicTerminalTab::appendOutput);
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
    m_output->appendPlainText(QStringLiteral("It is not xterm.js yet, so full-screen programs like htop/nano/vim are not expected to behave correctly.\n"));

    m_thread->start();
}

BasicTerminalTab::~BasicTerminalTab()
{
    disconnectShell();

    if (m_thread != nullptr) {
        m_thread->quit();
        m_thread->wait(3000);
    }
}

void BasicTerminalTab::sendCurrentInput()
{
    if (m_worker == nullptr || m_input == nullptr) {
        return;
    }

    const QString command = m_input->text();

    if (command.isEmpty()) {
        m_worker->sendInput(QStringLiteral("\n"));
        return;
    }

    m_worker->sendInput(command + QStringLiteral("\n"));
    m_input->clear();
}

void BasicTerminalTab::appendOutput(const QString &output)
{
    if (m_output == nullptr) {
        return;
    }

    m_output->moveCursor(QTextCursor::End);
    m_output->insertPlainText(output);
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

    appendOutput(QStringLiteral("\n[DD-SSH] ") + state + QStringLiteral("\n"));
}

void BasicTerminalTab::showWorkerError(const QString &error)
{
    appendOutput(QStringLiteral("\n[DD-SSH ERROR] ") + error + QStringLiteral("\n"));

    if (m_statusLabel != nullptr) {
        m_statusLabel->setText(QStringLiteral("Error: ") + error);
    }
}

void BasicTerminalTab::handleWorkerFinished()
{
    if (m_input != nullptr) {
        m_input->setEnabled(false);
    }

    if (m_sendButton != nullptr) {
        m_sendButton->setEnabled(false);
    }

    if (m_disconnectButton != nullptr) {
        m_disconnectButton->setEnabled(false);
    }

    appendOutput(QStringLiteral("\n[DD-SSH] Shell worker finished.\n"));
}

void BasicTerminalTab::disconnectShell()
{
    if (m_worker != nullptr) {
        m_worker->stop();
    }
}
