#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Set up Qt toolbar window
    ui->setupUi(this);
    ui->pointTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->pointEditorPanel->hide();
    ui->frameSlider->setEnabled(false);

    // Set window icon
    QPixmap logo = QPixmap(":/Logo/northern-red.png");
    //setWindowIcon(QIcon(logo));

    // Set up scene
    pen = QPen(QColor(50,205,50, 100), 1, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin);
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setBackgroundBrush(QBrush(Qt::black, Qt::SolidPattern));
    ui->insetView->setScene(scene);
    ui->insetView->setBackgroundBrush(QBrush(Qt::black, Qt::SolidPattern));

    // Set up frame
    image_item = new ImageItem();
    image_item->setPixmap(logo);
    scene->addItem(image_item);

    // Set up chart
    chart = new Chart;
    chart->setTitle("Dynamic spline chart");
    chart->legend()->hide();
    chart->setAnimationOptions(QChart::AllAnimations);
    chart_view = new QChartView(chart);
    chart_view->setRenderHint(QPainter::Antialiasing);
    chart_view->setFixedSize(300,400);
    ui->chartLayout->addWidget(chart_view);

    // Connect signals
    connect(image_item, SIGNAL(currentPositionRgbChanged(QPointF&)), this, SLOT(showMousePosition(QPointF&)));
    connect(image_item, SIGNAL(pixelClicked(QPointF&)), this, SLOT(onPixelClicked(QPointF&)));
    connect(ui->pointTable, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(on_pointTable_currentCellChanged(int,int,int,int)));
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
            removeAllSceneEllipses();
            removeAllSceneLines();
            drawCrosshair(x, y);

            // Update inset
            ui->insetView->centerOn(x,y);
        }
    }
}

void MainWindow::removeAllSceneEllipses()
{
    foreach (QGraphicsItem *item, scene->items())
    {
        QGraphicsEllipseItem *ellipse = qgraphicsitem_cast<QGraphicsEllipseItem *>(item);
        if (ellipse)
        {
            scene->removeItem(ellipse);
        }
    }
}

void MainWindow::removeAllSceneLines()
{
    foreach (QGraphicsItem *item, scene->items())
    {
        QGraphicsLineItem *line = qgraphicsitem_cast<QGraphicsLineItem *>(item);
        if (line)
        {
            scene->removeItem(line);
        }
    }
}

void MainWindow::drawCrosshair(int x, int y)
{
    scene->addEllipse( x-5, y-5, 10, 10, pen);
    scene->addLine(x-4, y, x+4, y, pen);
    scene->addLine(x, y-4, x, y+4, pen);
}

void MainWindow::savePointsToCSV(QString filename)
{
    QString text_data;
    int row_count = ui->pointTable->rowCount();
    text_data += QString("x,y,\n");
    for (int row=0; row<row_count; row++)
    {
        QTableWidgetItem* item = ui->pointTable->item(row, 0);
        if (item)
        {
            QStringList coordinate = item->text().split(",");
            text_data += coordinate[0];
            text_data += ",";
            text_data += coordinate[1];
            text_data += ",";
            text_data += "\n";
        }
    }
    QFile csv_file(filename);
    if(csv_file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {

        QTextStream out(&csv_file);
        out << text_data;

        csv_file.close();
    }
    qDebug() << "saved all points: " << filename;
}

void MainWindow::on_frameSpinBox_valueChanged(int arg1)
{
    qDebug() << "frame update.";
    int frame_index = arg1-1;
    cap.set(CV_CAP_PROP_POS_FRAMES, frame_index);
    cap.read(current_frame);
    img = QImage((uchar*) current_frame.data, current_frame.cols, current_frame.rows, current_frame.step, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(img);

    // Show in view, scaled to view bounds & keeping aspect ratio
    image_item->setPixmap(pixmap);

    // Determine where to place the ellipse based on the frame value and its associated (x,y) position
    removeAllSceneEllipses();
    removeAllSceneLines();
    QTableWidgetItem* item = ui->pointTable->item(frame_index, 0);
    if (item)
    {
        QStringList coordinate = item->text().split(",");
        int x = (coordinate[0]).toInt();
        int y = (coordinate[1]).toInt();
        drawCrosshair(x, y);

        // Update inset
        ui->insetView->centerOn(x,y);
    }

    // Determine what to track
    //cv::Tracker
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (!current_frame.empty())
    {
        img = QImage((uchar*) current_frame.data, current_frame.cols, current_frame.rows, current_frame.step, QImage::Format_RGB888);
        QPixmap pixmap = QPixmap::fromImage(img);

        // Show in view, scaled to view bounds & keeping aspect ratio
        image_item->setPixmap(pixmap);
        QRectF bounds = scene->itemsBoundingRect();
        ui->graphicsView->fitInView(bounds, Qt::KeepAspectRatio);
        ui->graphicsView->centerOn(0,0);
    }
}

void MainWindow::on_action_Open_triggered()
{       
    // load a video
    QString result = QFileDialog::getOpenFileName(this, tr("Open Video File 2"), "/home", tr("Video Files (*.avi)"));
    video_filepath = result.toUtf8().constData();
    QString video_filename = QString::fromStdString(video_filepath.substr(video_filepath.find_last_of("/\\") + 1));
    qDebug() << "filename: " << video_filename;

    cap = cv::VideoCapture(video_filepath);
    int frame_count = cap.get(CV_CAP_PROP_FRAME_COUNT);

    // update ui elements
    ui->frameSlider->setEnabled(true);
    ui->pointEditorPanel->show();
    ui->pointTable->setRowCount(frame_count);
    ui->frameSlider->setRange(1, frame_count);
    ui->frameSpinBox->setRange(1, frame_count);
    ui->videoComboBox->addItem(video_filename);

    // show frame zero
    cap.set(CV_CAP_PROP_POS_FRAMES, 0);
    cap.read(current_frame);
    img = QImage((uchar*) current_frame.data, current_frame.cols, current_frame.rows, current_frame.step, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(img);

    // Show in view, scaled to view bounds & keeping aspect ratio
    image_item->setPixmap(pixmap);
    QRectF bounds = scene->itemsBoundingRect();
    ui->graphicsView->fitInView(bounds, Qt::KeepAspectRatio);
    ui->graphicsView->centerOn(0,0);

    // 5X scale in inset view
    ui->insetView->scale(5,5);
    ui->insetView->centerOn(bounds.center());
}

void MainWindow::on_pointTable_currentCellChanged(int row, int column, int previous_row, int previous_column)
{
    // Update the frame based on the row selected
    QTableWidgetItem* item = ui->pointTable->item(row, column);
    ui->frameSpinBox->setValue(row+1);

    // Enable/disable the button for deleting points
    if (item && !item->text().trimmed().isEmpty())
    {
        ui->deletePointButton->setEnabled(true);
    }
    else
    {
        ui->deletePointButton->setEnabled(false);
    }
}

void MainWindow::on_saveButton_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this,
        tr("Save pixel positions"), "",
        tr("CSV (*.csv);;All Files (*)"));
    if (!filename.trimmed().isEmpty())
    {
        savePointsToCSV(filename);
    }
}
