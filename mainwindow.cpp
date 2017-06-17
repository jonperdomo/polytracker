#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHBoxLayout>
#include <QFileDialog>
#include <QString>
#include <QDebug>
#include <QImage>
#include <QTableWidgetItem>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Set up Qt toolbar window
    ui->setupUi(this);
    ui->pointTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->pointEditorPanel->hide();

    // Set window icon
    QPixmap logo = QPixmap(":/Logo/northern-red.png");
    //setWindowIcon(QIcon(logo));

    // Set up scene
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setBackgroundBrush(QBrush(Qt::black, Qt::SolidPattern));

    // Set up frame
    image_item = new ImageItem();
    image_item->setPixmap(logo);
    scene->addItem(image_item);

    // Connect signals
    connect(image_item, SIGNAL(currentPositionRgbChanged(QPointF&)), this, SLOT(showMousePosition(QPointF&)));
    connect(image_item, SIGNAL(pixelClicked(QPointF&)), this, SLOT(onPixelClicked(QPointF&)));
    connect(ui->pointTable, SIGNAL(cellClicked(int,int)), this, SLOT(on_pointTable_cellClicked(int,int)));
    show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showMousePosition(QPointF &pos)
{
    if (!current_frame.empty())
    {
        cv::Size mat_size = current_frame.size();
        int x = static_cast<int>(pos.x());
        int y = static_cast<int>(pos.y());
        if (x >= 0 && y >= 0 && x <= mat_size.width && y <= mat_size.height)
        {
            ui->mousePositionLabel->setText("x: " + QString::number(x) + ", y: " + QString::number(y));
        }
    }
}

void MainWindow::onPixelClicked(QPointF &pos)
{
    qDebug() << "entered here: " << pos.x() << ", "<< pos.y();
    if (!current_frame.empty())
    {
        cv::Size mat_size = current_frame.size();
        int x = static_cast<int>(pos.x());
        int y = static_cast<int>(pos.y());
        if (x >= 0 && y >= 0 && x <= mat_size.width && y <= mat_size.height)
        {
            int row = ui->frameSlider->value()-1;
            QString text = QString("%1, %2").arg(x).arg(y);
            ui->pointTable->setItem(row, 0, new QTableWidgetItem(text));
            //for (int row = 0; row < ui->pointTable->rowCount(); ++row)
            //{
            //    if (!ui->pointTable->item(row, 0))
            //    {
            //        QString text = QString("%1, %2").arg(x).arg(y);
            //        ui->pointTable->setItem(row, 0, new QTableWidgetItem(text));
            //        break;
            //    }
            //}
        }
    }
}

void MainWindow::on_frameSpinBox_valueChanged(int arg1)
{
    cap.set(CV_CAP_PROP_POS_FRAMES, arg1-1);
    cap.read(current_frame);
    img = QImage((uchar*) current_frame.data, current_frame.cols, current_frame.rows, current_frame.step, QImage::Format_RGB888);
    pixel = QPixmap::fromImage(img);

    // Show in view, scaled to view bounds & keeping aspect ratio
    image_item->setPixmap(pixel);
    QRectF bounds = scene->itemsBoundingRect();
    ui->graphicsView->fitInView(bounds, Qt::KeepAspectRatio);
    ui->graphicsView->centerOn(0,0);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (!current_frame.empty())
    {
        img = QImage((uchar*) current_frame.data, current_frame.cols, current_frame.rows, current_frame.step, QImage::Format_RGB888);
        pixel = QPixmap::fromImage(img);

        // Show in view, scaled to view bounds & keeping aspect ratio
        image_item->setPixmap(pixel);
        QRectF bounds = scene->itemsBoundingRect();
        ui->graphicsView->fitInView(bounds, Qt::KeepAspectRatio);
        ui->graphicsView->centerOn(0,0);
    }
}

void MainWindow::on_action_Open_triggered()
{       
    // load a video
    QString result;
    result = QFileDialog::getOpenFileName(this, tr("Open Video File 2"), "/home", tr("Video Files (*.avi)"));
    video_filepath = result.toUtf8().constData();
    QString video_filename = QString::fromStdString(video_filepath.substr(video_filepath.find_last_of("/\\") + 1));
    qDebug() << "filename: " << video_filename;

    cap = cv::VideoCapture(video_filepath);
    frame_count = cap.get(CV_CAP_PROP_FRAME_COUNT);

    // update ui elements
    ui->pointEditorPanel->show();
    ui->pointTable->setRowCount(frame_count);
    ui->frameSlider->setRange(1, frame_count);
    ui->frameSpinBox->setRange(1, frame_count);
    ui->videoComboBox->addItem(video_filename);

    // show frame zero
    cap.set(CV_CAP_PROP_POS_FRAMES, 0);
    cap.read(current_frame);
    img = QImage((uchar*) current_frame.data, current_frame.cols, current_frame.rows, current_frame.step, QImage::Format_RGB888);
    pixel = QPixmap::fromImage(img);

    // Show in view, scaled to view bounds & keeping aspect ratio
    image_item->setPixmap(pixel);
    QRectF bounds = scene->itemsBoundingRect();
    ui->graphicsView->fitInView(bounds, Qt::KeepAspectRatio);
    ui->graphicsView->centerOn(0,0);
}

void MainWindow::on_pointTable_cellClicked(int row, int column)
{
    qDebug() << "row: " << row << " column: " << column;
    QTableWidgetItem* item = ui->pointTable->item(row, column);

    // Update the frame based on the row selected
    ui->frameSpinBox->setValue(row+1);

    // Enable/disable the button for deleting points
    if (item && !item->text().trimmed().isEmpty())
    {
        ui->deletePointButton->setEnabled(true);
        qDebug() << "not empty: " << item->text();
    }
    else
    {
        ui->deletePointButton->setEnabled(false);
        qDebug() << "empty";
    }
}
