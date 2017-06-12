#include "mainwindow.h"
#include <QApplication>

#include <QDesktopWidget>
#include <QPoint>
#include <QRect>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
