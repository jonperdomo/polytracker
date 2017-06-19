#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <string>
#include <QImage>
#include <QPixmap>

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsView>

#include "imageitem.h"


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

private:
    Ui::MainWindow *ui;
    QLabel *video_label;
    std::string video_filepath;
    cv::VideoCapture cap;
    int frame_count;
    QImage img;
    QPixmap pixel;
    QPen pen;
    QGraphicsScene *scene;
    QGraphicsEllipseItem *ellipse;
    QGraphicsRectItem *rectangle;
    QGraphicsTextItem *text;
    QPixmap pix;
    ImageItem *image_item;
    QGraphicsEllipseItem *ellipse_item;
    void removeAllSceneEllipses();
    void removeAllSceneLines();
};

#endif // MAINWINDOW_H
