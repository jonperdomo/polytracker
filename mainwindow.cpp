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
    ContourList contours = frame_contours.at(frame_index);
    Hierarchy hierarchy = frame_hierarchies.at(frame_index);

    /// Draw contours
    cv::RNG rng(12345);
    if (frame_index > 1)
    {
        for (int i = 0; i< contours.size(); i++)
        {
            cv::Scalar color = cv::Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
            cv::drawContours(current_frame, contours, i, color, 1, 8, hierarchy, 0, cv::Point());
        }
    } else {
        std::vector<cv::Scalar> color_set = contour_colors.at(frame_index);
        for (int i=0; i< contours.size(); i++)
        {
            cv::Scalar color = color_set.at(i);
            cv::drawContours(current_frame, contours, i, color, 1, 8, hierarchy, 0, cv::Point());
        }
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
        ContourList initial_contours;
        cv::findContours(canny_output, initial_contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

        ContourList contours;
        contours.resize(initial_contours.size());
        for(int k=0; k < initial_contours.size(); k++)
        {
            cv::approxPolyDP(initial_contours.at(k), contours[k], 3, true);
            //cv::convexHull(initial_contours.at(k), contours[k], false, true);
        }

        frame_contours.push_back(initial_contours);
        frame_hierarchies.push_back(hierarchy);
    }
    qDebug() << "done contouring. least count: " << least_contours;

    /// Instead of comparing the first with all the next,
    /// Only compare the first with the second, the second with the third, etc.
    /// Better matches.
    //std::vector<std::tuple<int, double>> ctr_matches;
    ContourList left_ctr_set = frame_contours.at(0);
    ContourList right_ctr_set = frame_contours.at(1);
    ContourList right_ctr_match_set(right_ctr_set.size());
    for (int j=0; j<left_ctr_set.size(); j++)
    {
        std::vector<cv::Point> left_ctr = left_ctr_set.at(j);
        cv::Rect left_rect = cv::boundingRect(left_ctr);
        int match = -1;
        double best_intersect = 0;
        Contour right_ctr_match;
        for (int k=0; k<right_ctr_set.size(); k++)
        {
            Contour right_ctr = right_ctr_set.at(k);
            cv::Rect right_rect = cv::boundingRect(right_ctr);
            double area = (left_rect & right_rect).area();
            if (area > best_intersect)
            {
                best_intersect = area;
                //match = k;
                right_ctr_match = right_ctr;
            }
        }
        right_ctr_match_set[j] = right_ctr_match;
        //ctr_matches.push_back(std::tuple<int, double>(match, best_intersect));
    }
    frame_contours[1] = right_ctr_match_set;


    int max_size;
    if (left_ctr_set.size() > right_ctr_set.size())
    {
        max_size = left_ctr_set.size();
    } else {
        max_size = right_ctr_set.size();
    }
    std::vector<cv::Scalar> left_colors;
    std::vector<cv::Scalar> right_colors;
    for (int i=0; i< max_size; i++)
    {
        cv::Scalar color = cv::Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
        left_colors.push_back(color);
        right_colors.push_back(color);
    }
    contour_colors.push_back(left_colors);
    contour_colors.push_back(right_colors);
    //std::sort(ctr_matches.begin(), ctr_matches.end(), [](std::tuple<int, double> const& n1, std::tuple<int, double> const& n2) {return std::get<1>(n1) > std::get<1>(n2);});


    //while (ctr_matches.size() > min_size)
    //{
    //    ctr_matches.pop_back();
    //}
    //std::swap(ctr_matches[0], ctr_matches.back());

    qDebug() << "matches.";

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
