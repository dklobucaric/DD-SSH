#include "WebTerminalTab.h"
#include "TerminalBridge.h"
#include "ssh/SshShellWorker.h"

#include <QApplication>
#include <QClipboard>
#include <QHBoxLayout>
#include <QLabel>
#include <QMetaObject>
#include <QPushButton>
#include <QThread>
#include <QVBoxLayout>
#include <QUrl>
#include <QWebChannel>
#include <QWebEnginePage>
#include <QWebEngineView>

WebTerminalTab::WebTerminalTab(
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

    m_statusLabel = new QLabel(QStringLiteral("Starting web terminal for ") + target, this);
    layout->addWidget(m_statusLabel);

    m_view = new QWebEngineView(this);
    layout->addWidget(m_view, 1);

    auto *buttonLayout = new QHBoxLayout();

    m_interruptButton = new QPushButton(QStringLiteral("Ctrl+C"), this);
    m_interruptButton->setToolTip(QStringLiteral("Send Ctrl+C to the remote shell."));
    buttonLayout->addWidget(m_interruptButton);

    m_pasteButton = new QPushButton(QStringLiteral("Paste"), this);
    m_pasteButton->setToolTip(QStringLiteral("Paste clipboard text into the remote shell. Newlines are sent as Enter."));
    buttonLayout->addWidget(m_pasteButton);

    m_clearButton = new QPushButton(QStringLiteral("Clear local view"), this);
    m_clearButton->setToolTip(QStringLiteral("Clear the local web terminal view only."));
    buttonLayout->addWidget(m_clearButton);

    m_disconnectButton = new QPushButton(QStringLiteral("Disconnect"), this);
    buttonLayout->addWidget(m_disconnectButton);

    buttonLayout->addStretch(1);
    layout->addLayout(buttonLayout);

    m_bridge = new TerminalBridge(this);
    m_channel = new QWebChannel(this);
    m_channel->registerObject(QStringLiteral("terminalBridge"), m_bridge);
    m_view->page()->setWebChannel(m_channel);

    connect(m_bridge, &TerminalBridge::ready, this, &WebTerminalTab::startShell);
    connect(m_bridge, &TerminalBridge::inputReceived, this, [this](const QString &input) {
        sendToWorker(input);
    });

    connect(m_interruptButton, &QPushButton::clicked, this, &WebTerminalTab::sendInterrupt);
    connect(m_pasteButton, &QPushButton::clicked, this, &WebTerminalTab::pasteClipboard);
    connect(m_bridge, &TerminalBridge::pasteRequested, this, &WebTerminalTab::pasteClipboard);
    connect(m_clearButton, &QPushButton::clicked, this, &WebTerminalTab::clearTerminal);
    connect(m_disconnectButton, &QPushButton::clicked, this, &WebTerminalTab::disconnectShell);

    m_view->setHtml(terminalHtml(), QUrl(QStringLiteral("qrc:///")));
}

WebTerminalTab::~WebTerminalTab()
{
    disconnectShell();

    if (m_thread != nullptr) {
        m_thread->quit();
        m_thread->wait(3000);
    }
}

QString WebTerminalTab::terminalHtml() const
{
    const QString target =
        m_session.username
        + QStringLiteral("@")
        + m_session.host
        + QStringLiteral(":")
        + QString::number(m_session.port);

    return QStringLiteral(R"HTML(
<!doctype html>
<html>
<head>
<meta charset="utf-8">
<title>DD-SSH Web Terminal</title>
<script src="qrc:///qtwebchannel/qwebchannel.js"></script>
<style>
    :root {
        color-scheme: dark;
    }

    html, body {
        margin: 0;
        padding: 0;
        width: 100%;
        height: 100%;
        overflow: hidden;
        background: #0b0f14;
        color: #d7e0ea;
        font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, "Liberation Mono", monospace;
        font-size: 14px;
    }

    #header {
        box-sizing: border-box;
        padding: 8px 10px;
        border-bottom: 1px solid #223042;
        background: #111822;
        color: #9fb3c8;
        font-size: 12px;
        user-select: none;
    }

    #terminal {
        box-sizing: border-box;
        width: 100%;
        height: calc(100% - 35px);
        overflow: auto;
        white-space: pre-wrap;
        word-break: normal;
        padding: 12px;
        outline: none;
        caret-color: transparent;
        tab-size: 4;
    }

    #terminal:focus {
        box-shadow: inset 0 0 0 1px #2f6feb;
    }

    .dim {
        color: #75879b;
    }
</style>
</head>
<body>
    <div id="header">
        DD-SSH Web Terminal fallback for __TARGET__ · direct input/paste dispatch fixed · xterm.js renderer comes next
    </div>
    <div id="terminal" tabindex="0" spellcheck="false"></div>

<script>
(function () {
    const terminal = document.getElementById('terminal');
    let bridge = null;

    function stripAnsi(text) {
        if (!text) {
            return '';
        }

        return text
            // OSC sequences: ESC ] ... BEL or ESC \\
            .replace(/\x1b\][\s\S]*?(?:\x07|\x1b\\)/g, '')
            // CSI sequences: ESC [ ... final byte
            .replace(/\x1b\[[0-?]*[ -/]*[@-~]/g, '')
            // One-character ESC sequences
            .replace(/\x1b[@-Z\\-_]/g, '')
            // Keep CR from overprinting the fallback display
            .replace(/\r/g, '');
    }

    function appendOutput(text, clean = true) {
        const out = clean ? stripAnsi(text) : text;

        if (!out) {
            return;
        }

        terminal.appendChild(document.createTextNode(out));
        terminal.scrollTop = terminal.scrollHeight;
    }

    window.ddsshClearTerminal = function () {
        terminal.textContent = '';
        terminal.focus();
    };

    function sendInput(text) {
        if (!bridge || !text) {
            return;
        }

        bridge.sendInput(text);
    }

    function ctrlChar(key) {
        if (!key || key.length !== 1) {
            return null;
        }

        const code = key.toUpperCase().charCodeAt(0);

        if (code < 65 || code > 90) {
            return null;
        }

        return String.fromCharCode(code - 64);
    }

    terminal.addEventListener('keydown', function (event) {
        let input = null;

        if (event.ctrlKey && !event.altKey && !event.metaKey) {
            if (event.key && event.key.toLowerCase() === 'v') {
                event.preventDefault();

                if (bridge) {
                    bridge.requestPaste();
                }

                return;
            }

            input = ctrlChar(event.key);
        } else if (!event.altKey && !event.metaKey) {
            switch (event.key) {
                case 'Enter': input = '\n'; break;
                case 'Backspace': input = '\x7f'; break;
                case 'Tab': input = '\t'; break;
                case 'Escape': input = '\x1b'; break;
                case 'ArrowUp': input = '\x1b[A'; break;
                case 'ArrowDown': input = '\x1b[B'; break;
                case 'ArrowRight': input = '\x1b[C'; break;
                case 'ArrowLeft': input = '\x1b[D'; break;
                case 'Home': input = '\x1b[H'; break;
                case 'End': input = '\x1b[F'; break;
                case 'Delete': input = '\x1b[3~'; break;
                case 'Insert': input = '\x1b[2~'; break;
                case 'PageUp': input = '\x1b[5~'; break;
                case 'PageDown': input = '\x1b[6~'; break;
                default:
                    if (event.key.length === 1) {
                        input = event.key;
                    }
                    break;
            }
        }

        if (input !== null) {
            event.preventDefault();
            sendInput(input);
        }
    });

    terminal.addEventListener('paste', function (event) {
        event.preventDefault();

        const text = event.clipboardData ? event.clipboardData.getData('text') : '';

        if (text) {
            text = text.replace(/\r\n/g, '\n').replace(/\r/g, '\n');

            if (text.indexOf('\n') !== -1 && !text.endsWith('\n')) {
                text += '\n';
            }

            sendInput(text);
        }
    });

    terminal.addEventListener('mousedown', function () {
        terminal.focus();
    });

    appendOutput('DD-SSH web terminal channel test\n', false);
    appendOutput('Target: __TARGET__\n\n', false);
    appendOutput('This web terminal milestone captures keyboard input directly inside the terminal area.\n', false);
    appendOutput('Paste works with Ctrl+V or the Paste button. Multiline paste is sent directly to the remote shell.\n', false);
    appendOutput('It is still using a fallback renderer, not bundled xterm.js yet, so full-screen apps are not expected to be correct.\n\n', false);

    new QWebChannel(qt.webChannelTransport, function (channel) {
        bridge = channel.objects.terminalBridge;

        bridge.outputReceived.connect(function (text) {
            appendOutput(text, true);
        });

        bridge.statusChanged.connect(function (text) {
            appendOutput('\n[DD-SSH] ' + text + '\n', false);
        });

        bridge.errorReceived.connect(function (text) {
            appendOutput('\n[DD-SSH ERROR] ' + text + '\n', false);
        });

        bridge.terminalReady();
        terminal.focus();
    });
}());
</script>
</body>
</html>
)HTML").replace(QStringLiteral("__TARGET__"), target.toHtmlEscaped());
}

void WebTerminalTab::startShell()
{
    if (m_shellStarted) {
        return;
    }

    m_shellStarted = true;

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
    connect(m_worker, &SshShellWorker::outputReceived, m_bridge, &TerminalBridge::emitOutput);
    connect(m_worker, &SshShellWorker::stateChanged, this, [this](const QString &state) {
        if (m_statusLabel != nullptr) {
            m_statusLabel->setText(state);
        }

        if (m_bridge != nullptr) {
            m_bridge->emitStatus(state);
        }
    });
    connect(m_worker, &SshShellWorker::errorOccurred, m_bridge, &TerminalBridge::emitError);
    connect(m_worker, &SshShellWorker::finished, this, &WebTerminalTab::handleWorkerFinished);
    connect(m_worker, &SshShellWorker::finished, m_thread, &QThread::quit);
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_thread, &QThread::finished, this, [this]() {
        m_worker = nullptr;
    });

    m_thread->start();
}

void WebTerminalTab::sendToWorker(const QString &input)
{
    if (m_worker == nullptr || input.isEmpty()) {
        return;
    }

    // SshShellWorker::sendInput is intentionally thread-safe and only appends
    // to a mutex-protected queue. Do not use a queued Qt call here: the worker
    // thread is busy running the SSH read/write loop, so queued slots may not
    // be delivered until the shell exits. Direct dispatch keeps keyboard input
    // and paste responsive.
    m_worker->sendInput(input);
}

void WebTerminalTab::sendInterrupt()
{
    if (m_worker == nullptr) {
        return;
    }

    sendToWorker(QString(QChar(0x03)));

    if (m_bridge != nullptr) {
        m_bridge->emitStatus(QStringLiteral("Sent Ctrl+C to remote shell."));
    }

    if (m_view != nullptr) {
        m_view->setFocus();
    }
}

void WebTerminalTab::pasteClipboard()
{
    if (m_worker == nullptr) {
        return;
    }

    QClipboard *clipboard = QApplication::clipboard();
    if (clipboard == nullptr) {
        return;
    }

    QString text = clipboard->text();
    if (text.isEmpty()) {
        if (m_bridge != nullptr) {
            m_bridge->emitStatus(QStringLiteral("Clipboard is empty. Nothing pasted."));
        }

        if (m_view != nullptr) {
            m_view->setFocus();
        }

        return;
    }

    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QChar('\r'), QChar('\n'));

    const bool isMultiline = text.contains(QChar('\n'));
    if (isMultiline && !text.endsWith(QChar('\n'))) {
        text.append(QChar('\n'));
    }

    const int characterCount = text.size();
    const int lineCount = isMultiline
        ? text.count(QChar('\n'))
        : 1;

    sendToWorker(text);

    if (m_bridge != nullptr) {
        m_bridge->emitStatus(
            QStringLiteral("Sent clipboard text to remote shell (%1 chars, %2 line(s)).")
                .arg(characterCount)
                .arg(lineCount)
        );
    }

    if (m_view != nullptr) {
        m_view->setFocus();
    }
}

void WebTerminalTab::clearTerminal()
{
    if (m_view != nullptr) {
        m_view->page()->runJavaScript(QStringLiteral("window.ddsshClearTerminal && window.ddsshClearTerminal();"));
        m_view->setFocus();
    }
}

void WebTerminalTab::disconnectShell()
{
    if (m_worker != nullptr) {
        m_worker->stop();
    }
}

void WebTerminalTab::handleWorkerFinished()
{
    if (m_interruptButton != nullptr) {
        m_interruptButton->setEnabled(false);
    }

    if (m_disconnectButton != nullptr) {
        m_disconnectButton->setEnabled(false);
    }

    if (m_pasteButton != nullptr) {
        m_pasteButton->setEnabled(false);
    }

    if (m_bridge != nullptr) {
        m_bridge->emitStatus(QStringLiteral("Shell worker finished."));
    }

    if (m_statusLabel != nullptr) {
        m_statusLabel->setText(QStringLiteral("Shell finished."));
    }
}
