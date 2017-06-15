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

#include "imageitem.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Set up Qt toolbar window
    ui->setupUi(this);
    connect(ui->frameDisplay, SIGNAL(sendMousePosition(QPoint&)), this, SLOT(showMousePosition(QPoint&)));

    // Set up scene
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    QBrush greenBrush(Qt::green);
    QBrush blueBrush(Qt::blue);
    QPen outlinePen(Qt::black);
    outlinePen.setWidth(2);

    rectangle = scene->addRect(100, 0, 80, 100, outlinePen, blueBrush);

    // addEllipse(x,y,w,h,pen,brush)
    ellipse = scene->addEllipse(0, -100, 300, 60, outlinePen, greenBrush);

    text = scene->addText("bogotobogo.com", QFont("Arial", 20) );
    // movable text
    text->setFlag(QGraphicsItem::ItemIsMovable);

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

void MainWindow::showMousePosition(QPoint &pos)
{
    if (!current_frame.empty())
    {
        cv::Size mat_size = current_frame.size();
        int scaled_x = (float)pos.x() * ((float)mat_size.width / (float)(ui->frameDisplay->pixmap()->width()));
        int scaled_y = (float)pos.y() * ((float)mat_size.height / ((float)ui->frameDisplay->pixmap()->height()));
        ui->mousePositionLabel->setText("x: " + QString::number(pos.x()) + ", y: " + QString::number(pos.y()) + ", scaled-> " +  QString::number(scaled_x) + ", y: " + QString::number(scaled_y));
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
    ui->frameDisplay->setPixmap(pixel);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (!current_frame.empty())
    {
        img = QImage((uchar*) current_frame.data, current_frame.cols, current_frame.rows, current_frame.step, QImage::Format_RGB888);
        pixel = QPixmap::fromImage(img);
        int w = ui->frameDisplay->width();
        int h = ui->frameDisplay->height();
        ui->frameDisplay->setPixmap(pixel.scaled(w, h, Qt::KeepAspectRatio));
    }
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
    int w = ui->frameDisplay->width();
    int h = ui->frameDisplay->height();
    ui->frameDisplay->setPixmap(pixel.scaled(w, h, Qt::KeepAspectRatio));
}
