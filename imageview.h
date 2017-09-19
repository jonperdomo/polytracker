#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QGraphicsView>
#include <QWidget>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>
#include <QRubberBand>

class ImageView : public QGraphicsView
{
    Q_OBJECT;
public:
    ImageView(QWidget *parent=0);
    ~ImageView();

private:
    QRubberBand band;
    QPoint origin;
    QRect selection;
    bool selecting;

protected:
    void enterEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent* event);

signals:
    void selectionUpdate(QRect&);
    void pixelUpdate(QPointF&);
};

#endif // IMAGEVIEW_H
