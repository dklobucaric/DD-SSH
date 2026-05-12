#include <QApplication>
#include <QString>

#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setApplicationName("DD-SSH");
    QApplication::setApplicationDisplayName("DD-SSH");
    QApplication::setApplicationVersion(QStringLiteral(DD_SSH_VERSION_STRING));
    QApplication::setOrganizationName("DD-Lab");

    MainWindow window;
    window.show();

    return app.exec();
}
