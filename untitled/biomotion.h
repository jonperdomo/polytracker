#ifndef BIOMOTION_H
#define BIOMOTION_H

#include <QMainWindow>

namespace Ui {
class BioMotion;
}

class BioMotion : public QMainWindow
{
    Q_OBJECT

public:
    explicit BioMotion(QWidget *parent = 0);
    ~BioMotion();

private:
    Ui::BioMotion *ui;
};

#endif // BIOMOTION_H
