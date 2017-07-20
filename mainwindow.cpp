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
    if (frame_contours.size()>0)
    {
    cap.set(CV_CAP_PROP_POS_FRAMES, frame_index);
    cap.read(current_frame);




    ///------------------------------------------------------------


    int thresh = 100;
    int max_thresh = 255;
    cv::RNG rng(12345);

    /// Convert image to gray and blur it
    cv::Mat src_gray;
    cv::cvtColor(current_frame, src_gray, CV_BGR2GRAY);
    cv::blur(src_gray, src_gray, cv::Size(3,3));

    cv::Mat canny_output;
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    /// Detect edges using canny
    cv::Canny(src_gray, canny_output, thresh, thresh*2, 3);

    /// Find contours
    //cv::findContours(canny_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    contours = frame_contours.at(frame_index);
    hierarchy = frame_hierarchies.at(frame_index);

    /// Find bounding rects
    std::vector<cv::Rect> boundRect(contours.size());
    qDebug() << "cont size: " << contours.size();
    for (int i = 0; i< contours.size(); i++)
    {
        boundRect[i] = cv::boundingRect(contours.at(i));
    }

    std::vector<std::vector<std::vector<cv::Point>>> frame_contours;
    frame_contours.push_back(contours);

    /// Draw contours
    for (int i = 0; i< contours.size(); i++)
    {
        cv::Scalar color = cv::Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
        cv::drawContours(current_frame, frame_contours.at(0), i, color, 1, 8, hierarchy, 0, cv::Point());
    }

    ///------------------------------------------------------------

    /// Show in a window
    img = QImage((uchar*) current_frame.data, current_frame.cols, current_frame.rows, current_frame.step, QImage::Format_RGB888);
    QPixmap pixmap;
    pixmap = QPixmap::fromImage(img);

    /// Show in view, scaled to view bounds & keeping aspect ratio
    image_item->setPixmap(pixmap);

    /// Determine where to place the ellipse based on the frame value and its associated (x,y) position
    removeAllSceneEllipses();
    removeAllSceneLines();
    QTableWidgetItem* item = ui->pointTable->item(frame_index, 0);
    if (item)
    {
        QStringList coordinate = item->text().split(",");
        int x = (coordinate[0]).toInt();
        int y = (coordinate[1]).toInt();
        drawCrosshair(x, y);

        /// Update inset
        ui->insetView->centerOn(x,y);
    }
    }
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

    /// --------------------------------------------------------------------------------------------


    /// Find contours
    int thresh = 100;
    int max_thresh = 255;
    cv::RNG rng(12345);
    cv::Mat src_gray;
    cv::Mat canny_output;
    //std::vector<std::vector<std::vector<cv::Point>>> frame_contours;
    //std::vector<std::vector<cv::Vec<int, 4>>> frame_hierarchies;
    std::vector<std::vector<cv::Point> > frame_contour_centroids(frame_count);
    std::vector<std::vector<cv::Point> > tracked_contours;
    std::vector<cv::Vec4i> hierarchy;
    int least_contours = 200;
    for (int i=0; i<frame_count; i++)
    {
        cap.set(CV_CAP_PROP_POS_FRAMES, i);
        cap.read(current_frame);

        /// Convert image to gray and blur it
        cv::cvtColor(current_frame, src_gray, CV_BGR2GRAY);
        cv::blur(src_gray, src_gray, cv::Size(3,3));

        /// Detect edges using canny
        cv::Canny(src_gray, canny_output, thresh, thresh*2, 3);

        /// Find contours
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(canny_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
        frame_contours.push_back(contours);
        frame_hierarchies.push_back(hierarchy);

        /// Find mean of each contour
        std::vector<cv::Point> points;
        if (contours.size() < least_contours)
        {
            least_contours = contours.size();
        }
        for(int j=0; j<contours.size(); j++)
        {
            points = contours.at(j);
            cv::Point zero(0.0f, 0.0f);
            cv::Point sum  = std::accumulate(points.begin(), points.end(), zero);
            cv::Point mean_point(sum * (1.0f / points.size()));
            //qDebug() << "center: " << mean_point.x << ", " << mean_point.y;
            frame_contour_centroids[i].push_back(mean_point);
        }
    }
    qDebug() << "done contouring. least count: " << least_contours;

    //    /// Instead of comparing the first with all the next,
    //    /// Only compare the first with the second, the second with the third, etc.
    //    /// Better matches.
    //    std::vector<std::vector<int>> matches(frame_count);
    //    std::vector<cv::Point> first_set = frame_contour_centroids.at(0);

    //    for (int i=0; i<1; i++)
    //    {
    //        std::vector<std::vector<cv::Point>> first_contours = frame_contours.at(i);
    //        //cv::Point first_point = first_set.at(i);
    //        //std::vector<double> all_intersects;

    //        for (int j=0; j<first_contours.size(); j++)
    //        {
    //            std::vector<cv::Point> first_contour = first_contours.at(j);
    //            cv::Rect first_rect = cv::boundingRect(first_contour);
    //            std::vector<int> matches;
    //            for (int k=1; k<frame_count; k++)
    //            {
    //                std::vector<std::vector<cv::Point>> next_contours = frame_contours.at(k);
    //                int match = -1;
    //                double intersect_area = 0;
    //                //std::vector<double> intersects;
    //                for (int l=1; l<next_contours.size(); l++)
    //                {
    //                    std::vector<cv::Point> next_contour = next_contours.at(l);
    //                    cv::Rect rect = cv::boundingRect(next_contour);
    //                    double area = (first_rect & rect).area();
    //                    if (area > intersect_area)
    //                    {
    //                        intersect_area = area;
    //                        //intersects.push_back(area);
    //                        match = l;
    //                    }
    //                }
    //                matches.push_back(match);
    //                qDebug() << k << ": Best intersect = " << intersect_area;
    //            }
    //            qDebug() << j << " matches " << matches;
    //        }
    //        //qDebug() << j << " and " << j << "intersects: " << intersects;

    //    }


    /// --------------------------------------------------------------------------------------------


    /// show frame zero
    cap.set(CV_CAP_PROP_POS_FRAMES, 0);
    cap.read(current_frame);
    img = QImage((uchar*) current_frame.data, current_frame.cols, current_frame.rows, current_frame.step, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(img);

    /// Show in view, scaled to view bounds & keeping aspect ratio
    image_item->setPixmap(pixmap);
    QRectF bounds = scene->itemsBoundingRect();
    ui->graphicsView->fitInView(bounds, Qt::KeepAspectRatio);
    ui->graphicsView->centerOn(0,0);

    /// 5X scale in inset view
    ui->insetView->scale(5,5);
    ui->insetView->centerOn(bounds.center());

    /// Set up chart
    chart->axisX()->setRange(1, frame_count);    
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
