#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setApplicationName("DD-SSH");
    QApplication::setApplicationDisplayName("DD-SSH");
    QApplication::setOrganizationName("DD-Lab");

    QMainWindow window;
    window.setWindowTitle("DD-SSH");

    auto *central = new QWidget(&window);
    auto *layout = new QVBoxLayout(central);

auto *label = new QLabel(
    "DD-SSH foundation build\n\n"
    "Qt app skeleton is alive.\n"
    "Next milestone: terminal frontend + SSH backend.",
    central
);

    label->setAlignment(Qt::AlignCenter);

    layout->addWidget(label);
    window.setCentralWidget(central);

    auto *fileMenu = window.menuBar()->addMenu("&File");
    fileMenu->addAction("Exit", &window, &QMainWindow::close);

    window.statusBar()->showMessage("DD-SSH launch pad ready");
    window.resize(900, 600);
    window.show();

    return app.exec();
}
