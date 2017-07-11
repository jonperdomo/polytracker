#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "chart.h"
#include "imageitem.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <QDebug>
#include <QChartView>
#include <QMainWindow>
#include <QWidget>
#include <QFile>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QImage>
#include <QPen>
#include <QPixmap>
#include <QString>
#include <QTableWidgetItem>
#include <string>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    cv::Mat current_frame;
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void showMousePosition(QPointF& pos);
    void onPixelClicked(QPointF& pos);

private slots:
    void resizeEvent(QResizeEvent *event);
    void on_frameSpinBox_valueChanged(int arg1);
    void on_action_Open_triggered();
    void on_pointTable_currentCellChanged(int row, int column, int previous_row, int previous_column);
    void on_saveButton_clicked();

private:
    Ui::MainWindow *ui;
    std::string video_filepath;
    cv::VideoCapture cap;
    QImage img;
    QPen pen;
    QGraphicsScene *scene;
    ImageItem *image_item;
    Chart *chart;
    QChartView *chart_view;
    void removeAllSceneEllipses();
    void removeAllSceneLines();
    void drawCrosshair(int x, int y);
    void savePointsToCSV(QString filename);
};

#endif // MAINWINDOW_H
