#include "WebTerminalTab.h"
#include "TerminalBridge.h"
#include "ssh/SshShellWorker.h"

#include <QApplication>
#include <QClipboard>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QMetaObject>
#include <QPushButton>
#include <QShowEvent>
#include <QTimer>
#include <QThread>
#include <QVBoxLayout>
#include <QUrl>
#include <QVariant>
#include <QWebChannel>
#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QWebEngineView>


namespace {
QString javaScriptStringLiteral(const QString &value)
{
    QJsonArray array;
    array.append(value);

    const QString json = QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));

    if (json.size() >= 2) {
        return json.mid(1, json.size() - 2);
    }

    return QStringLiteral("\"\"");
}
}

WebTerminalTab::WebTerminalTab(
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
    ConfigManager config;
    m_appSettings = config.loadSettings();

    auto *layout = new QVBoxLayout(this);

    const QString target =
        m_session.username
        + QStringLiteral("@")
        + m_session.host
        + QStringLiteral(":")
        + QString::number(m_session.port);

    m_statusLabel = new QLabel(QStringLiteral("State: preparing terminal engine | Target: ") + target, this);
    layout->addWidget(m_statusLabel);

    m_view = new QWebEngineView(this);
    m_view->setFocusPolicy(Qt::StrongFocus);
    m_view->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);
    m_view->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    layout->addWidget(m_view, 1);

    auto *buttonLayout = new QHBoxLayout();

    m_interruptButton = new QPushButton(QStringLiteral("Copy"), this);
    m_interruptButton->setToolTip(QStringLiteral("Copy selected terminal text to the clipboard. Use keyboard Ctrl+C inside the terminal to interrupt remote programs."));
    buttonLayout->addWidget(m_interruptButton);

    m_pasteButton = new QPushButton(QStringLiteral("Paste"), this);
    m_pasteButton->setToolTip(QStringLiteral("Paste clipboard text into the remote shell. Newlines are sent as Enter."));
    buttonLayout->addWidget(m_pasteButton);

    m_clearButton = new QPushButton(QStringLiteral("Clear"), this);
    m_clearButton->setToolTip(QStringLiteral("Clear the local terminal view only. The remote shell is not affected."));
    buttonLayout->addWidget(m_clearButton);

    m_resetButton = new QPushButton(QStringLiteral("Reset"), this);
    m_resetButton->setToolTip(QStringLiteral("Reset the local xterm.js renderer state if a full-screen app leaves the terminal visually confused."));
    buttonLayout->addWidget(m_resetButton);

    m_focusButton = new QPushButton(QStringLiteral("Focus"), this);
    m_focusButton->setToolTip(QStringLiteral("Return keyboard focus to the terminal area."));
    buttonLayout->addWidget(m_focusButton);

    m_reconnectButton = new QPushButton(QStringLiteral("Reconnect"), this);
    m_reconnectButton->setToolTip(QStringLiteral("Reconnect this terminal using the same saved session."));
    m_reconnectButton->setEnabled(false);
    buttonLayout->addWidget(m_reconnectButton);

    m_disconnectButton = new QPushButton(QStringLiteral("Disconnect"), this);
    m_disconnectButton->setToolTip(QStringLiteral("Disconnect the active SSH shell session."));
    buttonLayout->addWidget(m_disconnectButton);

    buttonLayout->addStretch(1);
    layout->addLayout(buttonLayout);

    m_bridge = new TerminalBridge(this);
    m_channel = new QWebChannel(this);
    m_channel->registerObject(QStringLiteral("terminalBridge"), m_bridge);
    m_view->page()->setWebChannel(m_channel);

    connect(m_bridge, &TerminalBridge::ready, this, &WebTerminalTab::startShell);
    connect(m_bridge, &TerminalBridge::resizeRequested, this, &WebTerminalTab::requestPtyResize);
    connect(m_bridge, &TerminalBridge::inputReceived, this, [this](const QString &input) {
        sendToWorker(input);
    });

    connect(m_interruptButton, &QPushButton::clicked, this, &WebTerminalTab::copySelection);
    connect(m_pasteButton, &QPushButton::clicked, this, &WebTerminalTab::pasteClipboard);
    connect(m_bridge, &TerminalBridge::pasteRequested, this, &WebTerminalTab::pasteClipboard);
    connect(m_clearButton, &QPushButton::clicked, this, &WebTerminalTab::clearTerminal);
    connect(m_resetButton, &QPushButton::clicked, this, &WebTerminalTab::resetTerminal);
    connect(m_focusButton, &QPushButton::clicked, this, &WebTerminalTab::focusTerminal);
    connect(m_reconnectButton, &QPushButton::clicked, this, &WebTerminalTab::reconnectShell);
    connect(m_disconnectButton, &QPushButton::clicked, this, &WebTerminalTab::disconnectShell);

    connect(m_view, &QWebEngineView::loadStarted, this, [this]() {
        setConnectionUiState(QStringLiteral("loading terminal renderer"), false, false);
        emit lifecycleStatusChanged(QStringLiteral("Loading terminal renderer for ") + displayName());
    });

    connect(m_view, &QWebEngineView::loadProgress, this, [this](int progress) {
        if (m_statusLabel != nullptr && !m_shellStarted) {
            m_statusLabel->setText(
                QStringLiteral("State: loading terminal renderer (%1%) | Target: %2")
                    .arg(progress)
                    .arg(targetLabel())
            );
        }
    });

    connect(m_view, &QWebEngineView::loadFinished, this, [this](bool ok) {
        if (ok) {
            setConnectionUiState(QStringLiteral("terminal renderer ready"), false, false);
            emit lifecycleStatusChanged(QStringLiteral("Terminal renderer ready for ") + displayName());
            QTimer::singleShot(100, this, &WebTerminalTab::focusTerminal);
            return;
        }

        setConnectionUiState(QStringLiteral("terminal renderer failed"), false, false);
        emit lifecycleStatusChanged(QStringLiteral("Terminal renderer failed for ") + displayName());
    });

    setConnectionUiState(QStringLiteral("preparing terminal engine"), false, false);

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

bool WebTerminalTab::hasActiveShell() const
{
    return m_shellActive && m_worker != nullptr;
}

QString WebTerminalTab::displayName() const
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

QString WebTerminalTab::trafficSessionName() const
{
    return displayName();
}

qint64 WebTerminalTab::receivedBytesTotal() const
{
    return m_receivedBytesTotal;
}

qint64 WebTerminalTab::sentBytesTotal() const
{
    return m_sentBytesTotal;
}

QString WebTerminalTab::targetLabel() const
{
    return m_session.username
        + QStringLiteral("@")
        + m_session.host
        + QStringLiteral(":")
        + QString::number(m_session.port);
}

void WebTerminalTab::setConnectionUiState(const QString &state, bool remoteInputEnabled, bool reconnectAvailable)
{
    const QString normalizedState = state.trimmed().isEmpty()
        ? QStringLiteral("unknown")
        : state.trimmed();

    if (m_statusLabel != nullptr) {
        m_statusLabel->setText(
            QStringLiteral("State: %1 | Target: %2")
                .arg(normalizedState, targetLabel())
        );
    }

    if (m_interruptButton != nullptr) {
        m_interruptButton->setEnabled(remoteInputEnabled);
    }

    if (m_pasteButton != nullptr) {
        m_pasteButton->setEnabled(remoteInputEnabled);
    }

    if (m_disconnectButton != nullptr) {
        m_disconnectButton->setEnabled(remoteInputEnabled);
    }

    if (m_reconnectButton != nullptr) {
        m_reconnectButton->setEnabled(reconnectAvailable);
    }

    updateTerminalConnectionState(normalizedState);
}

void WebTerminalTab::updateTerminalConnectionState(const QString &state)
{
    if (m_view == nullptr) {
        return;
    }

    m_view->page()->runJavaScript(
        QStringLiteral("window.ddsshSetTerminalConnectionState && window.ddsshSetTerminalConnectionState(%1);")
            .arg(javaScriptStringLiteral(state))
    );
}

void WebTerminalTab::requestDisconnect()
{
    disconnectShell();
}

void WebTerminalTab::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    QTimer::singleShot(50, this, &WebTerminalTab::focusTerminal);
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
<title>DD-SSH xterm.js Terminal</title>
<script src="qrc:///qtwebchannel/qwebchannel.js"></script>
<link rel="stylesheet" href="qrc:///xterm/xterm.css">
<script src="qrc:///xterm/xterm.js"></script>
<script src="qrc:///xterm/addon-fit.js"></script>
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
        font-size: __FONT_SIZE__px;
    }

    #header {
        box-sizing: border-box;
        padding: 8px 10px;
        border-bottom: 1px solid #223042;
        background: #111822;
        color: #9fb3c8;
        font-size: 12px;
        user-select: none;
        white-space: nowrap;
        overflow: hidden;
        text-overflow: ellipsis;
    }

    #startupNotice {
        box-sizing: border-box;
        width: 100%;
        height: calc(100% - 35px);
        padding: 24px;
        background: #0b0f14;
        color: #d7e0ea;
        outline: none;
        user-select: none;
    }

    #startupNotice .title {
        font-weight: 700;
        color: #ffffff;
        margin-bottom: 10px;
    }

    #startupNotice .hint {
        color: #9fb3c8;
        line-height: 1.45;
        max-width: 780px;
    }

    #terminalHost {
        box-sizing: border-box;
        width: 100%;
        height: calc(100% - 35px);
        background: #0b0f14;
        outline: none;
        display: none;
    }

    #fallbackTerminal {
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
        display: none;
    }

    #fallbackTerminal:focus, #terminalHost:focus-within {
        box-shadow: inset 0 0 0 1px #2f6feb;
    }

    .xterm {
        height: 100%;
        padding: 8px;
        box-sizing: border-box;
    }
</style>
</head>
<body>
    <div id="header">
        DD-SSH Andromeda terminal for __TARGET__ · Renderer: loading · State: preparing terminal engine · PTY resize: on
    </div>
    <div id="startupNotice">
        <div class="title">Starting DD-SSH terminal engine…</div>
        <div class="hint">
            Preparing the local xterm.js renderer and Qt WebEngine bridge for __TARGET__.<br>
            On Windows, the first terminal tab can take a few seconds while Qt WebEngine initializes.
            Later terminal tabs usually open much faster.
        </div>
    </div>
    <div id="terminalHost"></div>
    <div id="fallbackTerminal" tabindex="0" spellcheck="false"></div>

<script>
(function () {
    const terminalHost = document.getElementById('terminalHost');
    const fallbackTerminal = document.getElementById('fallbackTerminal');
    const startupNotice = document.getElementById('startupNotice');
    let bridge = null;
    let term = null;
    let fitAddon = null;
    let usingXterm = false;
    let terminalInputEnabled = true;
    let rendererStatus = 'loading';
    let connectionState = 'starting';
    let lastReportedCols = 0;
    let lastReportedRows = 0;
    let fitTimer = null;

    window.ddsshGetTerminalSelection = function () {
        try {
            if (usingXterm && term && typeof term.hasSelection === 'function' && term.hasSelection()) {
                return term.getSelection() || '';
            }

            const selection = window.getSelection ? window.getSelection() : null;
            if (selection) {
                return selection.toString() || '';
            }
        } catch (error) {
            return '';
        }

        return '';
    };

    function updateHeader() {
        const header = document.getElementById('header');
        if (header) {
            header.textContent = 'DD-SSH Andromeda terminal for __TARGET__ · Renderer: ' + rendererStatus + ' · State: ' + connectionState + ' · PTY resize: on';
        }
    }

    function setRendererStatus(status) {
        rendererStatus = status || rendererStatus;
        updateHeader();
    }

    function setConnectionState(state) {
        connectionState = state || connectionState;
        updateHeader();
    }

    function hideStartupNotice() {
        if (startupNotice) {
            startupNotice.style.display = 'none';
        }
    }

    function reportTerminalSize(columns, rows) {
        if (!bridge || !columns || !rows) {
            return;
        }

        if (columns === lastReportedCols && rows === lastReportedRows) {
            return;
        }

        lastReportedCols = columns;
        lastReportedRows = rows;
        bridge.terminalResized(columns, rows);
    }

    function fitAndReport() {
        if (!usingXterm || !term || !fitAddon) {
            return;
        }

        try {
            fitAddon.fit();
            reportTerminalSize(term.cols, term.rows);
        } catch (error) {
            // Fit can fail briefly while Qt WebEngine is still laying out the page.
            // A later resize/focus event will retry.
        }
    }

    function scheduleFitAndReport(delayMs = 50) {
        if (fitTimer) {
            clearTimeout(fitTimer);
        }

        fitTimer = setTimeout(function () {
            fitTimer = null;
            fitAndReport();
        }, delayMs);
    }

    function stripAnsi(text) {
        if (!text) {
            return '';
        }

        return text
            .replace(/\x1b\][\s\S]*?(?:\x07|\x1b\\)/g, '')
            .replace(/\x1b\[[0-?]*[ -/]*[@-~]/g, '')
            .replace(/\x1b[@-Z\\-_]/g, '')
            .replace(/\r/g, '');
    }

    function fallbackAppend(text, clean = true) {
        const out = clean ? stripAnsi(text) : text;

        if (!out) {
            return;
        }

        fallbackTerminal.appendChild(document.createTextNode(out));
        fallbackTerminal.scrollTop = fallbackTerminal.scrollHeight;
    }

    function writeTerminal(text, cleanFallback = true) {
        if (!text) {
            return;
        }

        if (usingXterm && term) {
            term.write(text);
            return;
        }

        fallbackAppend(text, cleanFallback);
    }

    function sendInput(text) {
        if (!bridge || !text || !terminalInputEnabled) {
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

    function setupFallbackInput() {
        hideStartupNotice();
        terminalHost.style.display = 'none';
        fallbackTerminal.style.display = 'block';
        reportTerminalSize(80, 24);

        fallbackTerminal.addEventListener('keydown', function (event) {
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

        fallbackTerminal.addEventListener('paste', function (event) {
            event.preventDefault();

            let text = event.clipboardData ? event.clipboardData.getData('text') : '';

            if (text) {
                text = text.replace(/\r\n/g, '\n').replace(/\r/g, '\n');

                if (text.indexOf('\n') !== -1 && !text.endsWith('\n')) {
                    text += '\n';
                }

                sendInput(text);
            }
        });

        fallbackTerminal.addEventListener('mousedown', function () {
            fallbackTerminal.focus();
        });
    }

    function setupXterm() {
        if (typeof Terminal === 'undefined') {
            setRendererStatus('fallback - local resource missing');
            setupFallbackInput();
            fallbackAppend('DD-SSH xterm.js terminal channel test\n', false);
            fallbackAppend('Target: __TARGET__\n\n', false);
            fallbackAppend('Bundled xterm.js assets could not be loaded, so DD-SSH is using the fallback renderer.\n', false);
            fallbackAppend('This usually means the Qt resource bundle did not include resources/xterm assets correctly.\n\n', false);
            return;
        }

        usingXterm = true;
        setRendererStatus('xterm.js local');
        hideStartupNotice();
        fallbackTerminal.style.display = 'none';
        terminalHost.style.display = 'block';

        term = new Terminal({
            cursorBlink: true,
            convertEol: true,
            scrollback: 5000,
            fontFamily: __FONT_FAMILY_JS__,
            fontSize: __FONT_SIZE__,
            theme: {
                background: '#0b0f14',
                foreground: '#d7e0ea',
                cursor: '#ffffff',
                selectionBackground: '#264f78'
            }
        });

        if (typeof FitAddon !== 'undefined' && FitAddon.FitAddon) {
            fitAddon = new FitAddon.FitAddon();
            term.loadAddon(fitAddon);
        }

        term.open(terminalHost);
        fitAndReport();
        term.focus();

        term.writeln('DD-SSH Andromeda terminal');
        term.writeln('Target: __TARGET__');
        term.writeln('');
        term.writeln('Local xterm.js renderer, FitAddon and SSH PTY resize are active.');
        term.writeln('Use Reset only if a fullscreen app leaves the local terminal visually confused.');
        term.writeln('');

        term.onData(function (data) {
            sendInput(data);
        });

        terminalHost.addEventListener('mousedown', function () {
            if (term) {
                term.focus();
                scheduleFitAndReport(10);
            }
        });

        window.addEventListener('resize', function () {
            scheduleFitAndReport(50);
        });

        if (typeof ResizeObserver !== 'undefined') {
            const observer = new ResizeObserver(function () {
                scheduleFitAndReport(50);
            });
            observer.observe(terminalHost);
        }

        setTimeout(function () { fitAndReport(); }, 50);
        setTimeout(function () { fitAndReport(); }, 250);
        setTimeout(function () { fitAndReport(); }, 750);
    }

    window.ddsshSetTerminalInputEnabled = function (enabled) {
        terminalInputEnabled = !!enabled;

        if (!terminalInputEnabled) {
            setConnectionState('disconnected');
        }
    };

    window.ddsshSetTerminalConnectionState = function (state) {
        setConnectionState(state || 'unknown');
    };

    window.ddsshFocusTerminal = function () {
        if (usingXterm && term) {
            scheduleFitAndReport(10);
            term.focus();
            return;
        }

        fallbackTerminal.focus();
    };

    window.ddsshClearTerminal = function () {
        if (usingXterm && term) {
            term.clear();
            scheduleFitAndReport(10);
            term.focus();
            return;
        }

        fallbackTerminal.textContent = '';
        fallbackTerminal.focus();
    };

    window.ddsshResetTerminal = function () {
        if (usingXterm && term) {
            term.reset();
            scheduleFitAndReport(10);
            term.focus();
            return;
        }

        fallbackTerminal.textContent = '';
        fallbackTerminal.focus();
    };

    document.body.addEventListener('mousedown', function () {
        window.ddsshFocusTerminal();
    });

    new QWebChannel(qt.webChannelTransport, function (channel) {
        bridge = channel.objects.terminalBridge;

        bridge.outputReceived.connect(function (text) {
            writeTerminal(text, true);
        });

        bridge.statusChanged.connect(function (text) {
            writeTerminal('\r\n[DD-SSH] ' + text + '\r\n', false);
        });

        bridge.errorReceived.connect(function (text) {
            writeTerminal('\r\n[DD-SSH ERROR] ' + text + '\r\n', false);
        });

        setupXterm();
        fitAndReport();
        bridge.terminalReady();
        window.ddsshFocusTerminal();
        setTimeout(function () { window.ddsshFocusTerminal(); }, 50);
        setTimeout(function () { window.ddsshFocusTerminal(); }, 250);
    });
}());
</script>
</body>
</html>
)HTML")
        .replace(QStringLiteral("__TARGET__"), target.toHtmlEscaped())
        .replace(QStringLiteral("__FONT_FAMILY_JS__"), javaScriptStringLiteral(m_appSettings.terminalFontFamily.trimmed().isEmpty()
            ? QStringLiteral("monospace")
            : m_appSettings.terminalFontFamily.trimmed()))
        .replace(QStringLiteral("__FONT_SIZE__"), QString::number(m_appSettings.terminalFontSize));
}

void WebTerminalTab::startShell()
{
    if (m_shellStarted) {
        return;
    }

    m_shellStarted = true;
    m_shellActive = true;
    m_disconnectRequested = false;
    m_receivedBytesTotal = 0;
    m_sentBytesTotal = 0;
    emit trafficCountersChanged();
    setTerminalInputEnabled(false);
    setConnectionUiState(QStringLiteral("connecting"), false, false);

    emit tabTitleChanged(displayName() + QStringLiteral(" ●"));
    emit lifecycleStatusChanged(QStringLiteral("Connecting ") + displayName());

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

    if (m_lastTerminalColumns > 0 && m_lastTerminalRows > 0) {
        m_worker->resizePty(m_lastTerminalColumns, m_lastTerminalRows);
    }

    m_worker->moveToThread(m_thread);

    connect(m_thread, &QThread::started, m_worker, &SshShellWorker::start);
    connect(m_worker, &SshShellWorker::outputReceived, m_bridge, &TerminalBridge::emitOutput);
    connect(m_worker, &SshShellWorker::trafficUpdated, this, [this](qint64 receivedBytes, qint64 sentBytes) {
        m_receivedBytesTotal = receivedBytes;
        m_sentBytesTotal = sentBytes;
        emit trafficCountersChanged();
    });
    connect(m_worker, &SshShellWorker::stateChanged, this, [this](const QString &state) {
        QString uiState = state.trimmed();
        bool allowRemoteInput = false;

        if (state.contains(QStringLiteral("Disconnected"), Qt::CaseInsensitive)) {
            uiState = QStringLiteral("disconnected");
        } else if (state.contains(QStringLiteral("Disconnecting"), Qt::CaseInsensitive)) {
            uiState = QStringLiteral("disconnecting");
        } else if (state.contains(QStringLiteral("connection lost"), Qt::CaseInsensitive)) {
            uiState = QStringLiteral("connection lost");
        } else if (state.contains(QStringLiteral("closed"), Qt::CaseInsensitive)) {
            uiState = QStringLiteral("remote closed");
        } else if (state.startsWith(QStringLiteral("Connected"), Qt::CaseInsensitive)) {
            uiState = QStringLiteral("connected");
            allowRemoteInput = true;
            emit tabTitleChanged(displayName() + QStringLiteral(" ●"));
        } else if (state.contains(QStringLiteral("Authenticating"), Qt::CaseInsensitive)) {
            uiState = QStringLiteral("authenticating");
        } else if (state.contains(QStringLiteral("Connecting"), Qt::CaseInsensitive)) {
            uiState = QStringLiteral("connecting");
        } else if (state.contains(QStringLiteral("Opening"), Qt::CaseInsensitive)) {
            uiState = QStringLiteral("opening shell");
        }

        setConnectionUiState(uiState, allowRemoteInput, false);
        setTerminalInputEnabled(allowRemoteInput);
        emit lifecycleStatusChanged(state);

        if (m_bridge != nullptr) {
            m_bridge->emitStatus(state);
        }
    });
    connect(m_worker, &SshShellWorker::errorOccurred, m_bridge, &TerminalBridge::emitError);
    QThread *threadForThisRun = m_thread;
    SshShellWorker *workerForThisRun = m_worker;

    connect(m_worker, &SshShellWorker::finished, this, &WebTerminalTab::handleWorkerFinished);
    connect(m_worker, &SshShellWorker::finished, m_thread, &QThread::quit);
    connect(m_thread, &QThread::finished, workerForThisRun, &QObject::deleteLater);
    connect(m_thread, &QThread::finished, threadForThisRun, &QObject::deleteLater);
    connect(m_thread, &QThread::finished, this, [this, threadForThisRun]() {
        if (m_thread == threadForThisRun) {
            m_thread = nullptr;
            m_worker = nullptr;

            if (!m_shellActive) {
                setConnectionUiState(QStringLiteral("disconnected"), false, true);
            }
        }
    });

    m_thread->start();
}

void WebTerminalTab::requestPtyResize(int columns, int rows)
{
    if (columns <= 0 || rows <= 0) {
        return;
    }

    m_lastTerminalColumns = columns;
    m_lastTerminalRows = rows;

    if (m_worker != nullptr) {
        m_worker->resizePty(columns, rows);
    }
}

void WebTerminalTab::sendToWorker(const QString &input)
{
    if (!m_shellActive || m_worker == nullptr || input.isEmpty()) {
        return;
    }

    // SshShellWorker::sendInput is intentionally thread-safe and only appends
    // to a mutex-protected queue. Do not use a queued Qt call here: the worker
    // thread is busy running the SSH read/write loop, so queued slots may not
    // be delivered until the shell exits. Direct dispatch keeps keyboard input
    // and paste responsive.
    m_worker->sendInput(input);
}

void WebTerminalTab::copySelection()
{
    if (m_view == nullptr || m_view->page() == nullptr) {
        return;
    }

    m_view->page()->runJavaScript(
        QStringLiteral("window.ddsshGetTerminalSelection ? window.ddsshGetTerminalSelection() : ''"),
        [this](const QVariant &result) {
            const QString selectedText = result.toString();

            if (selectedText.isEmpty()) {
                if (m_bridge != nullptr) {
                    m_bridge->emitStatus(QStringLiteral("No terminal selection to copy."));
                }
                focusTerminal();
                return;
            }

            QClipboard *clipboard = QApplication::clipboard();
            if (clipboard != nullptr) {
                clipboard->setText(selectedText);
            }

            if (m_bridge != nullptr) {
                m_bridge->emitStatus(QStringLiteral("Copied selected terminal text to clipboard."));
            }

            focusTerminal();
        }
    );
}

void WebTerminalTab::pasteClipboard()
{
    if (!m_shellActive || m_worker == nullptr) {
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

        focusTerminal();

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

    focusTerminal();
}

void WebTerminalTab::clearTerminal()
{
    if (m_view != nullptr) {
        m_view->page()->runJavaScript(QStringLiteral("window.ddsshClearTerminal && window.ddsshClearTerminal();"));
    }

    focusTerminal();
}

void WebTerminalTab::resetTerminal()
{
    if (m_view != nullptr) {
        m_view->page()->runJavaScript(QStringLiteral("window.ddsshResetTerminal && window.ddsshResetTerminal();"));
    }

    if (m_bridge != nullptr) {
        m_bridge->emitStatus(QStringLiteral("Reset local terminal renderer state."));
    }

    focusTerminal();
}

void WebTerminalTab::focusTerminal()
{
    if (m_view == nullptr) {
        return;
    }

    m_view->setFocus(Qt::OtherFocusReason);
    m_view->page()->runJavaScript(QStringLiteral("window.ddsshFocusTerminal && window.ddsshFocusTerminal();"));
}

void WebTerminalTab::disconnectShell()
{
    if (m_worker != nullptr) {
        if (!m_disconnectRequested) {
            m_disconnectRequested = true;

            if (m_bridge != nullptr) {
                m_bridge->emitStatus(QStringLiteral("Disconnect requested."));
            }

            setConnectionUiState(QStringLiteral("disconnecting"), false, false);

            emit lifecycleStatusChanged(QStringLiteral("Disconnect requested for ") + displayName());
        }

        m_worker->stop();
    }
}

void WebTerminalTab::reconnectShell()
{
    if (m_shellActive) {
        if (m_bridge != nullptr) {
            m_bridge->emitStatus(QStringLiteral("Reconnect ignored because the SSH terminal is already connected."));
        }

        focusTerminal();
        return;
    }

    if (m_worker != nullptr || (m_thread != nullptr && m_thread->isRunning())) {
        if (m_bridge != nullptr) {
            m_bridge->emitStatus(QStringLiteral("Waiting for the previous SSH worker to finish before reconnecting."));
        }

        focusTerminal();
        return;
    }

    if (m_bridge != nullptr) {
        m_bridge->emitStatus(QStringLiteral("Reconnect requested. Starting a new SSH shell session..."));
    }

    setConnectionUiState(QStringLiteral("reconnecting"), false, false);

    m_disconnectRequested = false;
    m_shellStarted = false;
    startShell();
    focusTerminal();
}

void WebTerminalTab::setTerminalInputEnabled(bool enabled)
{
    if (m_view == nullptr) {
        return;
    }

    m_view->page()->runJavaScript(
        enabled
            ? QStringLiteral("window.ddsshSetTerminalInputEnabled && window.ddsshSetTerminalInputEnabled(true);")
            : QStringLiteral("window.ddsshSetTerminalInputEnabled && window.ddsshSetTerminalInputEnabled(false);")
    );
}

void WebTerminalTab::handleWorkerFinished()
{
    m_shellActive = false;
    m_shellStarted = false;
    setTerminalInputEnabled(false);
    setConnectionUiState(QStringLiteral("disconnected"), false, false);
    emit tabTitleChanged(displayName() + QStringLiteral(" ×"));
    emit lifecycleStatusChanged(QStringLiteral("Disconnected: ") + displayName());

    if (m_bridge != nullptr) {
        m_bridge->emitStatus(QStringLiteral("Shell worker finished. Terminal is disconnected."));
    }


}
