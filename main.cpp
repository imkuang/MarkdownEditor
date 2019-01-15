#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("Markdown Editor");

    MainWindow w;
    w.show();

    return a.exec();
}
