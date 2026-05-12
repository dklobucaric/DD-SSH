#include "MainWindow.h"

#include <QLabel>
#include <QMenuBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("DD-SSH");

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);

    auto *label = new QLabel(
        "DD-SSH foundation build\n\n"
        "Qt app skeleton is alive.\n"
        "MainWindow class is now separated.\n\n"
        "Next milestone: terminal frontend + SSH backend.",
        central
    );

    label->setAlignment(Qt::AlignCenter);

    layout->addWidget(label);
    setCentralWidget(central);

    auto *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("Exit", this, &QWidget::close);

    statusBar()->showMessage("DD-SSH launch pad ready");

    resize(900, 600);
}
