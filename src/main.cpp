#include <QApplication>
#include <QIcon>
#include <QString>

#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setApplicationName("DD-SSH");
    QApplication::setApplicationDisplayName("DD-SSH");
    QApplication::setApplicationVersion(QStringLiteral(DD_SSH_VERSION_STRING));
    QApplication::setOrganizationName("DD-LAB");
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/dd-ssh.png")));

    MainWindow window;
    window.show();

    return app.exec();
}
