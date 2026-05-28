#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>

class TerminalBridge : public QObject
{
    Q_OBJECT

public:
    explicit TerminalBridge(QObject *parent = nullptr);

    void emitOutput(const QByteArray &output);
    void emitStatus(const QString &status);
    void emitError(const QString &error);

public slots:
    void sendInput(const QString &input);
    void requestPaste();
    void terminalReady();
    void terminalResized(int columns, int rows);

signals:
    void inputReceived(const QString &input);
    void pasteRequested();
    void ready();
    void resizeRequested(int columns, int rows);
    void outputBytesReceived(const QString &base64Output);
    void statusChanged(const QString &status);
    void errorReceived(const QString &error);
};
