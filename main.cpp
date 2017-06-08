#include "mainwindow.h"
#include <QApplication>

#include <QDesktopWidget>
#include <QPoint>
#include <QRect>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QRect r = w.geometry();
    QPoint desktop_center = QApplication::desktop()->availableGeometry().center();
    desktop_center.setX(desktop_center.x()*.5);
    //desktop_center.setY(desktop_center.y()*1.75);
    r.moveCenter(desktop_center);
    w.setGeometry(r);
    w.show();

    return a.exec();
}
