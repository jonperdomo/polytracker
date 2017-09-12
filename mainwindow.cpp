#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    blur(3),
    threshold(200)
{
    /// Set up Qt toolbar window
    ui->setupUi(this);
    ui->contourTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->contourTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->contourPanel->hide();
    ui->frameSlider->setEnabled(false);
    ui->frameSpinBox->setEnabled(false);

    /// Set window icon
    QPixmap logo = QPixmap(":/Logo/northern-red.png");
    QPixmap icon = QPixmap(":/Logo/northern-red-icon.png");
    setWindowIcon(QIcon(icon));

    /// Set up scene
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setBackgroundBrush(QBrush(Qt::black, Qt::SolidPattern));
    ui->insetView->setScene(scene);
    ui->insetView->setBackgroundBrush(QBrush(Qt::black, Qt::SolidPattern));

    /// Set up frame
    image_item = new ImageItem();
    image_item->setPixmap(logo);
    scene->addItem(image_item);

    /// Set up blur, threshold spinboxes
    ui->blurSpinBox->setValue(blur);
    ui->thresholdSpinBox->setValue(threshold);

    //// Set up chart
    //chart = new Chart;
    //chart->setTitle("Dynamic spline chart");
    //chart->legend()->hide();
    //chart->setAnimationOptions(QChart::AllAnimations);
    //chart_view = new QChartView(chart);
    //chart_view->setRenderHint(QPainter::Antialiasing);
    //chart_view->setFixedSize(300,400);
    //ui->chartLayout->addWidget(chart_view);

    /// Connect signals
    connect(image_item, SIGNAL(currentPositionRgbChanged(QPointF&)), this, SLOT(showMousePosition(QPointF&)));
    connect(image_item, SIGNAL(pixelClicked(QPointF&)), this, SLOT(showClosestContour(QPointF&)));
    connect(image_item, SIGNAL(pixelClicked(QPointF&)), this, SLOT(onPixelClicked(QPointF&)));
    connect(ui->contourTable, SIGNAL(itemSelectionChanged()), this, SLOT(on_contourTable_itemSelectionChanged()));
    connect(ui->contourTable, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(on_contourTable_currentCellChanged(int,int,int,int)));
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

void MainWindow::showClosestContour(QPointF &pos)
{
    if (ui->mainTabs->currentIndex()==0 && ui->contoursCheckBox->isChecked() && frame_centroids.size()>0)
    {
        cv::Point mouse;
        mouse.x = pos.x();
        mouse.y = pos.y();
        int reach = 15;
        int best_distance = current_frame.cols;
        int index = -1;
        int frame_index = cap.get(CV_CAP_PROP_POS_FRAMES)-1;
        std::vector<cv::Point> centroids = frame_centroids.at(frame_index);
        for (int i=0; i<centroids.size(); i++)
        {
            cv::Point centroid = centroids.at(i);
            double dist = cv::norm(centroid-mouse);

            if (dist < best_distance)
            {
                best_distance = dist;
                index = i;
            }
        }
        if (best_distance < reach)
        {
            ui->contourTable->selectRow(index);
            drawAllContours(frame_index);
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

void MainWindow::drawCrosshair(int x, int y, QColor color)
{
    QPen pen = QPen(color, 1, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin);
    scene->addEllipse( x-5, y-5, 10, 10, pen);
    scene->addLine(x-4, y, x+4, y, pen);
    scene->addLine(x, y-4, x, y-1, pen);
    scene->addLine(x, y+1, x, y+4, pen);
}

void MainWindow::savePointsToCSV(QString filename)
{
    QString text_data;
    text_data += QString("Centroid points,\nFrame,Contour,X,Y,\n");
    qDebug() << "frame centroids size: " << frame_centroids.size();
    for (int frame=0; frame<frame_centroids.size(); frame++)
    {
        /// Populate the CSV
        std::vector<cv::Point> centroids = frame_centroids.at(frame);
        for (int contour=0; contour<centroids.size(); contour++)
        {
            cv::Point centroid = centroids.at(contour);
            text_data += QString("%1,%2,%3,%4,\n").arg(frame+1).arg(contour+1).arg(centroid.x).arg(centroid.y);
            //qDebug() << QString("%1,%2,%3,%4,\n").arg(frame+1).arg(contour+1).arg(centroid.x).arg(centroid.y);
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

void MainWindow::drawAllContours(int frame_index)
{
    if (frame_contours.size()>0)
    {
        /// Remove the previous crosshairs
        removeAllSceneEllipses();
        removeAllSceneLines();

        /// Set the frame
        cap.set(CV_CAP_PROP_POS_FRAMES, frame_index);
        cap.read(current_frame);

        /// Update the table's centroid values
        std::vector<cv::Point> centroids = frame_centroids.at(frame_index);
        for (int i=0; i<centroids.size(); i++)
        {
            cv::Point centroid = centroids.at(i);
            QString text = QString("%1, %2").arg(centroid.x).arg(centroid.y);
            ui->contourTable->setItem(i, 1, new QTableWidgetItem(text));
        }

        /// Get the current selected contours
        QItemSelectionModel *selection = ui->contourTable->selectionModel();
        std::vector<int> inds;
        foreach (QModelIndex index, selection->selectedRows())
        {
            inds.push_back(index.row());
        }

        /// Draw contours
        if (ui->contoursCheckBox->isChecked())
        {
            cv::RNG rng(12345);
            ContourList contours = frame_contours.at(frame_index);
            Hierarchy hierarchy = frame_hierarchies.at(frame_index);
            for (int i=0; i<contour_colors.size(); i++)
            {
                cv::Scalar color = contour_colors.at(i);
                if (std::find(inds.begin(), inds.end(), i) != inds.end())
                {
                    inds.erase(std::remove(inds.begin(), inds.end(), i), inds.end());
                    cv::drawContours(current_frame, contours, i, color, 2, 8, hierarchy, 0, cv::Point());
                } else {
                    cv::drawContours(current_frame, contours, i, color, 1, 8, hierarchy, 0, cv::Point());
                }
            }
        }

        /// Draw centroids
        if (ui->centroidsCheckBox->isChecked())
        {
            std::vector<cv::Point> centroids = frame_centroids.at(frame_index);
            for (int i=0; i<centroids.size(); i++)
            {
                cv::Point centroid = centroids.at(i);
                cv::Scalar color = contour_colors.at(i);
                drawCrosshair(centroid.x, centroid.y, QColor(color.val[0], color.val[1], color.val[2], 150));
            }
        }

        /// Show in a window
        img = QImage((uchar*) current_frame.data, current_frame.cols, current_frame.rows, current_frame.step, QImage::Format_RGB888);
        QPixmap pixmap;
        pixmap = QPixmap::fromImage(img);

        /// Show in view, scaled to view bounds & keeping aspect ratio
        image_item->setPixmap(pixmap);
    }
}

void MainWindow::showCannyFrame(int frame_index)
{
    if (cap.isOpened())
    {
        /// Clear all the crosshairs
        removeAllSceneEllipses();
        removeAllSceneLines();

        /// Set the frame
        cap.set(CV_CAP_PROP_POS_FRAMES, frame_index);
        cap.read(current_frame);

        try
        {
            /// Convert image to gray and blur it
            int blur = ui->blurSpinBox->value();
            int threshold = ui->thresholdSpinBox->value();
            cv::RNG rng(12345);
            cv::Mat src_gray;
            cv::Mat canny_output;
            cv::cvtColor(current_frame, src_gray, CV_BGR2GRAY);
            cv::blur(src_gray, src_gray, cv::Size(blur,blur));

            /// Detect edges using canny
            cv::Canny(src_gray, canny_output, threshold/2, threshold, 3);

            /// Show in a window
            img = QImage((uchar*) canny_output.data, canny_output.cols, canny_output.rows, canny_output.step, QImage::Format_Grayscale8);
            QPixmap pixmap;
            pixmap = QPixmap::fromImage(img);

            /// Show in view, scaled to view bounds & keeping aspect ratio
            image_item->setPixmap(pixmap);
        }
        catch (const std::runtime_error& error)
        {
            qDebug() << "blur, threshold combination failed!";
        }
    }
}

cv::Point MainWindow::getMeanPoint(const Contour contour)
{
    cv::Point zero(0.0f, 0.0f);
    cv::Point sum  = std::accumulate(contour.begin(), contour.end(), zero);
    cv::Point mean_point = (sum * (1.0f / contour.size()));
    return mean_point;
}

cv::Point MainWindow::getCenterOfMass(const Contour contour)
{
    cv::Moments mu = cv::moments(contour);
    cv::Point centroid = cv::Point(mu.m10/mu.m00 , mu.m01/mu.m00);
    if (centroid.x < 0 || centroid.y < 0)
    {
        return getMeanPoint(contour);
    } else {
        return centroid;
    }
}

void MainWindow::updateAllContours()
{
    /// Clear current contours
    frame_centroids.clear();
    frame_contours.clear();
    frame_hierarchies.clear();
    contour_colors.clear();

    /// Find contours
    int frame_count = cap.get(CV_CAP_PROP_FRAME_COUNT);
    frame_centroids.resize(frame_count);
    frame_contours.resize(frame_count);
    frame_hierarchies.resize(frame_count);

    int blur = ui->blurSpinBox->value();
    int threshold = ui->thresholdSpinBox->value();
    cv::RNG rng(12345);
    cv::Mat src_gray;
    cv::Mat canny_output;
    ContourListSet initial_contours;
    HierarchyListSet initial_hierarchies;
    std::vector<std::vector<cv::Point>> initial_centroids(frame_count);
    for (int i=0; i<frame_count; i++)
    {
        cap.set(CV_CAP_PROP_POS_FRAMES, i);
        cap.read(current_frame);

        /// Convert image to gray and blur it
        cv::cvtColor(current_frame, src_gray, CV_BGR2GRAY);
        cv::blur(src_gray, src_gray, cv::Size(blur,blur));

        /// Detect edges using canny
        cv::Canny(src_gray, canny_output, threshold/2, threshold, 3);

        /// Find contours
        std::vector<cv::Vec4i> hierarchy;
        ContourList contours;
        cv::findContours(canny_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
        initial_contours.push_back(contours);
        initial_hierarchies.push_back(hierarchy);

        /// Find centroid (mean) of each contour
        for(int j=0; j<(int)contours.size(); j++)
        {
            Contour contour = contours.at(j);
            initial_centroids[i].push_back(getMeanPoint(contour));
        }
    }
    contour_colors.resize(initial_contours.at(0).size());
    qDebug() << "Completed finding contours";

    for (int c=0; c<(int)contour_colors.size(); c++)
    {
        /// Match first contour
        frame_contours.at(0).push_back(initial_contours.at(0).at(c));
        frame_hierarchies.at(0).push_back(initial_hierarchies.at(0).at(c));
        frame_centroids.at(0).push_back(initial_centroids.at(0).at(c));
        std::vector<cv::Point> first_set = initial_centroids.at(0);
        cv::Point first_point = first_set.at(c);
        for (int i=1; i<(int)frame_count; i++)
        {
            std::vector<cv::Point> point_set = initial_centroids.at(i);
            int best_distance = current_frame.cols;
            int index = -1;
            for (int k=0; k<(int)point_set.size(); k++)
            {
                cv::Point point = point_set.at(k);
                double dist = cv::norm(first_point-point);

                if (dist < best_distance)
                {
                    best_distance = dist;
                    index = k;
                }
            }
            first_point = point_set.at(index);
            frame_contours.at(i).push_back(initial_contours.at(i).at(index));
            frame_hierarchies.at(i).push_back(initial_hierarchies.at(i).at(index));
            frame_centroids.at(i).push_back(initial_centroids.at(i).at(index));
        }
        /// Set the color for the contour
        cv::Scalar color = cv::Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
        contour_colors[c] = (color);

        /// Add contours to the table
        ui->contourTable->insertRow(ui->contourTable->rowCount());
        ui->contourTable->setItem(ui->contourTable->rowCount()-1, 0, new QTableWidgetItem());
        ui->contourTable->item(ui->contourTable->rowCount()-1, 0)->setBackgroundColor(QColor(color.val[0], color.val[1], color.val[2], 255));
    }
    qDebug() << "Found" << contour_colors.size() << "contours";
}

void MainWindow::on_frameSpinBox_valueChanged(int arg1)
{
    int frame_index = arg1-1;
    int tab = ui->mainTabs->currentIndex();
    if (tab == 0)
    {
        drawAllContours(frame_index);
    }
    else if (tab == 1)
    {
        showCannyFrame(frame_index);
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (!current_frame.empty())
    {
        QRectF bounds = scene->itemsBoundingRect();
        ui->graphicsView->fitInView(bounds, Qt::KeepAspectRatio);
        ui->graphicsView->centerOn(0,0);
    }
}

void MainWindow::on_action_Open_triggered()
{       


    /// Load a video
    QString result = QFileDialog::getOpenFileName(this, tr("Select a Video File"), "/home", tr("Video Files (*.avi *.mp4 *.m4v)"));
    video_filepath = result.toUtf8().constData();
    QString video_filename = QString::fromStdString(video_filepath.substr(video_filepath.find_last_of("/\\") + 1));

    if (!video_filename.isEmpty())
    {
        /// Clear current contours
        frame_centroids.clear();
        frame_contours.clear();
        frame_hierarchies.clear();
        contour_colors.clear();

        /// Remove the previous crosshairs
        removeAllSceneEllipses();
        removeAllSceneLines();

        cap = cv::VideoCapture(video_filepath);
        qDebug() << "opened video: " << video_filename;

        /// Enable video control elements, update elements with video information.
        int frame_count = cap.get(CV_CAP_PROP_FRAME_COUNT);
        ui->contourPanel->show();
        ui->videoComboBox->addItem(video_filename);

        /// Get contours
        updateAllContours();

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
        ui->insetView->setFixedHeight(250);

        ///// Set up chart
        //chart->axisX()->setRange(1, frame_count);

        /// Enable frame sliders
        ui->frameSlider->setEnabled(true);
        ui->frameSlider->setRange(1, frame_count);
        ui->frameSlider->setValue(1);
        ui->frameSpinBox->setEnabled(true);
        ui->frameSpinBox->setRange(1, frame_count);
        ui->frameSpinBox->setValue(1);
    }
}

void MainWindow::on_contourTable_itemSelectionChanged()
{
    QItemSelectionModel *selection = ui->contourTable->selectionModel();
    ui->deleteContourButton->setEnabled(selection->hasSelection());
}

void MainWindow::on_contourTable_currentCellChanged(int row, int column, int previous_row, int previous_column)
{
    /// Outline the contour selected in the table
    int frame_index = cap.get(CV_CAP_PROP_POS_FRAMES)-1;
    drawAllContours(frame_index);
}

void MainWindow::on_deleteContourButton_clicked()
{
    qDebug() << "delete button was pressed.";
    QItemSelectionModel *selection = ui->contourTable->selectionModel();
    int row;
    int shift = 0;

    foreach (QModelIndex index, selection->selectedRows())
    {
        row = index.row() - shift;
        ui->contourTable->removeRow(row);

        /// Erase the contour in each frame
        for (int i=0; i<frame_contours.size(); i++)
        {
            frame_contours[i].erase(frame_contours[i].begin() + row);
            frame_hierarchies[i].erase(frame_hierarchies[i].begin() + row);
        }
        contour_colors.erase(contour_colors.begin() + row);
        shift++;
    }
    on_frameSpinBox_valueChanged(ui->frameSpinBox->value());
}

void MainWindow::on_findContoursButton_clicked()
{
    /// TODO: Check if contours have been updated (deleted) before doing this.
    updateAllContours();
    on_frameSpinBox_valueChanged(ui->frameSpinBox->value());
}

void MainWindow::on_contoursCheckBox_stateChanged(int arg1)
{
    /// Outline the contour selected in the table
    int frame_index = cap.get(CV_CAP_PROP_POS_FRAMES)-1;
    drawAllContours(frame_index);
}

void MainWindow::on_centroidsCheckBox_stateChanged(int arg1)
{
    /// Outline the contour selected in the table
    int frame_index = cap.get(CV_CAP_PROP_POS_FRAMES)-1;
    drawAllContours(frame_index);
}

void MainWindow::on_actionSave_to_CSV_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this,
        tr("Save all contour centroids to CSV"), "",
        tr("CSV (*.csv);;All Files (*)"));
    if (!filename.trimmed().isEmpty())
    {
        savePointsToCSV(filename);
    }
}

void MainWindow::on_mainTabs_currentChanged(int index)
{
    int frame_index = cap.get(CV_CAP_PROP_POS_FRAMES)-1;
    if (index==0)
    {
        drawAllContours(frame_index);
    }
    else if (index==1)
    {
        showCannyFrame(frame_index);
    }
}

void MainWindow::on_trackingApplyButton_clicked()
{
    int frame_index = cap.get(CV_CAP_PROP_POS_FRAMES)-1;
    updateAllContours();
    showCannyFrame(frame_index);
}

void MainWindow::on_blurSpinBox_valueChanged(int arg1)
{
    int frame_index = cap.get(CV_CAP_PROP_POS_FRAMES)-1;
    showCannyFrame(frame_index);
}

void MainWindow::on_thresholdSpinBox_valueChanged(int arg1)
{
    int frame_index = cap.get(CV_CAP_PROP_POS_FRAMES)-1;
    showCannyFrame(frame_index);
}
