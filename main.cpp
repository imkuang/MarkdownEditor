#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("Markdown Editor");

    QTranslator translator;
    translator.load(":/i18n/zh_CN");
    a.installTranslator(&translator);

    MainWindow w;
    w.show();

    return a.exec();
}
