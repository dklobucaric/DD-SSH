#include <QApplication>

#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setApplicationName("DD-SSH");
    QApplication::setApplicationDisplayName("DD-SSH");
    QApplication::setOrganizationName("DD-Lab");

    MainWindow window;
    window.show();

    return app.exec();
}
