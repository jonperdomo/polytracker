#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHBoxLayout>
#include <QFileDialog>
#include <QString>
#include <QDebug>
#include <QImage>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Set up Qt toolbar window
    ui->setupUi(this);
    show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::mouse_callback(int event, int x, int y, int flags, void* userdata)
{
    MainWindow* main_window = reinterpret_cast<MainWindow*>(userdata);
    main_window->do_mouse(event, x, y);
}

void MainWindow::do_mouse(int event, int x, int y)
{
    if  ( event == cv::EVENT_LBUTTONDOWN )
    {
        qDebug() << "Left button of the mouse is clicked - position (" << x << ", " << y << ")";
        cv::Mat clone = current_frame.clone();
        if (clone.data == NULL)
        {
            qDebug() << "empty frame";

        } else {
            int radius = 8;
            cv::circle(clone, cv::Point(x,y), radius, cv::Scalar(212, 175, 123), 1);
            cv::line(clone, cv::Point(x-radius, y), cv::Point(x+radius, y), cv::Scalar(212, 175, 123), 1);
            cv::line(clone, cv::Point(x, y-radius), cv::Point(x, y+radius), cv::Scalar(212, 175, 123), 1);
            cv::imshow("BioMotion [Video]", clone); //show the frame in "BioMotion [Video]" window
            cv::waitKey();
        }

    }
    else if  ( event == cv::EVENT_RBUTTONDOWN )
    {
        qDebug() << "Right button of the mouse is clicked - position (" << x << ", " << y << ")";
    }
    else if  ( event == cv::EVENT_MBUTTONDOWN )
    {
        qDebug() << "Middle button of the mouse is clicked - position (" << x << ", " << y << ")";
    }
    else if ( event == cv::EVENT_MOUSEMOVE )
    {
        qDebug() << "Mouse move over the window - position (" << x << ", " << y << ")";
    }
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
        cap.read(frame);
        if (frame.data == NULL) //if empty frame, break loop
        {
        break;
        }
        current_frame = frame;
        frame_index = cap.get(CV_CAP_PROP_POS_FRAMES);
        ui->frameSlider->setValue(frame_index);
        cv::imshow("BioMotion [Video]", current_frame); //show the frame in "BioMotion [Video]" window
    cv::waitKey(30);
    }
}

void MainWindow::on_frameSpinBox_valueChanged(int arg1)
{
    cap.set(CV_CAP_PROP_POS_FRAMES, arg1);
    cap.read(current_frame);
    img = QImage((uchar*) current_frame.data, current_frame.cols, current_frame.rows, current_frame.step, QImage::Format_RGB888);
    pixel = QPixmap::fromImage(img);
    ui->frameLabel->setPixmap(pixel);
    //cv::imshow("BioMotion [Video]", current_frame); //show the frame in "BioMotion [Video]" window
}

void MainWindow::on_action_Open_triggered()
{       
    // load a video
    QString result;
    result = QFileDialog::getOpenFileName(this, tr("Open Video File 2"), "/home", tr("Video Files (*.avi)"));
    video_filepath = result.toUtf8().constData();
    cap = cv::VideoCapture(video_filepath);
    frame_count = cap.get(CV_CAP_PROP_FRAME_COUNT);

    // update ui elements
    ui->frameSlider->setRange(0, frame_count-1);
    ui->frameSpinBox->setRange(0, frame_count-1);

    // Set up OpenCV window
    cv::namedWindow("BioMotion [Video]", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL);
    cv::setMouseCallback("BioMotion [Video]", mouse_callback, this); // Pass the class instance pointer here

    // show frame zero
    cap.set(CV_CAP_PROP_POS_FRAMES, 0);    
    cap.read(current_frame);
    img = QImage((uchar*) current_frame.data, current_frame.cols, current_frame.rows, current_frame.step, QImage::Format_RGB888);
    pixel = QPixmap::fromImage(img);
    ui->frameLabel->setPixmap(pixel);
    //cv::imshow("BioMotion [Video]", current_frame); //show the frame in "BioMotion [Video]" window
    //cv::waitKey();
}
