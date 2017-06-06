#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHBoxLayout>
#include <QFileDialog>
#include <QString>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_playButton_clicked()
{
    int frame_index = cap.get(CV_CAP_PROP_POS_FRAMES);
    if (frame_index == frame_count) {
        cap.set(CV_CAP_PROP_POS_FRAMES, 0);
    }
    play();
}

void MainWindow::play()
{
    int frame_index;
    while(1)
    {
        cv::Mat frame;
        bool bSuccess = cap.read(frame);
        if (!bSuccess) //if not success, break loop
        {
        break;
        }
        frame_index = cap.get(CV_CAP_PROP_POS_FRAMES);
        ui->frameSlider->setValue(frame_index);
        cv::imshow("BioMotion [Video]", frame); //show the frame in "BioMotion [Video]" window
    cv::waitKey(30);
    }
}

void MainWindow::on_frameSpinBox_valueChanged(int arg1)
{
    cap.set(CV_CAP_PROP_POS_FRAMES, arg1);
    cv::Mat frame;
    cap.read(frame);
    cv::imshow("BioMotion [Video]", frame); //show the frame in "BioMotion [Video]" window
}

void MainWindow::on_action_Open_triggered()
{
    // load a video
    QString result;
    result = QFileDialog::getOpenFileName(this, tr("Open Video File 2"), "/home", tr("Video Files (*.avi)"));
    video_filepath = result.toUtf8().constData();
    cap = cv::VideoCapture(video_filepath);
    cv::namedWindow("BioMotion [Video]", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL);
    frame_count = cap.get(CV_CAP_PROP_FRAME_COUNT);

    // update ui elements
    ui->frameSlider->setRange(0, frame_count-1);
    ui->frameSpinBox->setRange(0, frame_count-1);

    // show frame zero
    cap.set(CV_CAP_PROP_POS_FRAMES, 0);
    cv::Mat frame;
    cap.read(frame);
    cv::imshow("BioMotion [Video]", frame); //show the frame in "BioMotion [Video]" window
}
