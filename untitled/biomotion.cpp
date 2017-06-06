#include "biomotion.h"
#include "ui_biomotion.h"

BioMotion::BioMotion(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BioMotion)
{
    ui->setupUi(this);
}

BioMotion::~BioMotion()
{
    delete ui;
}
