#pragma once

#include <QObject>
#include <QString>

class TerminalBridge : public QObject
{
    Q_OBJECT

public:
    explicit TerminalBridge(QObject *parent = nullptr);

    void emitOutput(const QString &output);
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
    void outputReceived(const QString &output);
    void statusChanged(const QString &status);
    void errorReceived(const QString &error);
};
