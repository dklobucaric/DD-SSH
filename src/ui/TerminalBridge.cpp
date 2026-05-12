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

void TerminalBridge::emitOutput(const QString &output)
{
    emit outputReceived(output);
}

void TerminalBridge::emitStatus(const QString &status)
{
    emit statusChanged(status);
}

void TerminalBridge::emitError(const QString &error)
{
    emit errorReceived(error);
}
