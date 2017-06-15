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
    void play();
    void do_mouse(int event, int x, int y);
    void showMousePosition(QPoint& pos);
    static void mouse_callback(int event, int x, int y, int flags, void* userdata);

private slots:
    void resizeEvent(QResizeEvent *event);
    void on_playButton_clicked();
    void on_frameSpinBox_valueChanged(int arg1);
    void on_action_Open_triggered();

private:
    Ui::MainWindow *ui;
    QLabel *video_label;
    std::string video_filepath;
    cv::VideoCapture cap;
    int frame_count;
    QImage img;
    QPixmap pixel;

    QGraphicsScene *scene;
    QGraphicsEllipseItem *ellipse;
    QGraphicsRectItem *rectangle;
    QGraphicsTextItem *text;
    QPixmap pix;
};

#endif // MAINWINDOW_H
