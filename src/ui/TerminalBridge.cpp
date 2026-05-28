#include "TerminalBridge.h"

TerminalBridge::TerminalBridge(QObject *parent)
    : QObject(parent)
{
}

void TerminalBridge::sendInput(const QString &input)
{
    emit inputReceived(input);
}

void TerminalBridge::requestPaste()
{
    emit pasteRequested();
}

void TerminalBridge::terminalReady()
{
    emit ready();
}

void TerminalBridge::terminalResized(int columns, int rows)
{
    emit resizeRequested(columns, rows);
}

void TerminalBridge::emitOutput(const QByteArray &output)
{
    if (output.isEmpty()) {
        return;
    }

    emit outputBytesReceived(QString::fromLatin1(output.toBase64()));
}

void TerminalBridge::emitStatus(const QString &status)
{
    emit statusChanged(status);
}

void TerminalBridge::emitError(const QString &error)
{
    emit errorReceived(error);
}
