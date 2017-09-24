#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "chart.h"
#include "imageitem.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/video/tracking.hpp>
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

typedef std::vector<cv::Point> Contour;
typedef std::vector<Contour> ContourList;
typedef std::vector<ContourList> ContourListSet;

typedef std::vector<cv::Vec<int, 4>> Hierarchy;
typedef std::vector<Hierarchy> HierarchyListSet;

typedef std::vector<cv::Scalar> ColorList;
//typedef std::vector<std::vector<cv::Vec<int, 4>>> HierarchyListSet;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    cv::Mat current_frame;
    cv::Mat blurred_frame;
    cv::Rect2d roi;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void showMousePosition(QPointF& pos);
    //void showClosestContour(QPointF& pos);
    void updateInset(QPointF& pos);
    void showSelectedContours(QRect& selection);
    void deselectAll();

private slots:
    void resizeEvent(QResizeEvent *event);
    void on_frameSpinBox_valueChanged(int arg1);
    void on_action_Open_triggered();
    void on_contourTable_itemSelectionChanged();
    void on_contourTable_currentCellChanged(int row, int column, int previous_row, int previous_column);
    void on_deleteContourButton_clicked();
    void on_findContoursButton_clicked();
    void on_contoursCheckBox_stateChanged(int arg1);
    void on_centroidsCheckBox_stateChanged(int arg1);
    void on_actionSave_to_CSV_triggered();
    void on_mainTabs_currentChanged(int index);
    void on_trackingApplyButton_clicked();
    void on_blurSpinBox_valueChanged(int arg1);
    void on_thresholdSpinBox_valueChanged(int arg1);

private:
    Ui::MainWindow *ui;
    std::string video_filepath;
    cv::VideoCapture cap;
    std::vector<std::tuple<int, double>> ctr_matches;
    std::vector<std::vector<cv::Point>> frame_centroids;
    ContourListSet frame_contours;
    HierarchyListSet frame_hierarchies;
    ColorList contour_colors;
    std::vector<int> active_contours;
    QImage img;
    QGraphicsScene *scene;
    ImageItem *image_item;
    Chart *chart;
    QChartView *chart_view;
    int blur;
    int threshold;
    void removeAllSceneEllipses();
    void removeAllSceneLines();
    void drawCrosshair(int x, int y, QColor color=QColor(50,205,50,100));
    void savePointsToCSV(QString filename);
    void updateAllContours();
    void drawAllContours(int frame_index);
    void showCannyFrame(int frame_index);
    cv::Point getMeanPoint(const Contour contour);
    cv::Point getCenterOfMass(const Contour contour);
};

#endif // MAINWINDOW_H
