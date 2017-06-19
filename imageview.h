#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QGraphicsView>
#include <QWidget>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QWheelEvent>

class ImageView : public QGraphicsView
{
    Q_OBJECT;
public:
    ImageView(QWidget *parent=0);
    ~ImageView();

protected:
    void enterEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent* event);
};

#endif // IMAGEVIEW_H
