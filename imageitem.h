#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include <QObject>
#include <QWidget>
#include <QGraphicsPixmapItem>
#include <QPoint>

#include <QMouseEvent>

class ImageItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    ImageItem(QObject *parent = 0);
    ~ImageItem();

protected:
    void mouseMoveEvent(QMouseEvent *mouse_event);

signals:
    void currentPositionRgbChanged(QPoint pos, QPoint rgb);

public slots:
};

#endif // IMAGEITEM_H
