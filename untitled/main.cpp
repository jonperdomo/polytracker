#include "biomotion.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BioMotion w;
    w.show();

    return a.exec();
}
