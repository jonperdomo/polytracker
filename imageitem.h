#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include <QObject>
#include <QWidget>
#include <QGraphicsPixmapItem>
#include <QPointF>

#include <QGraphicsSceneMouseEvent>

class ImageItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    ImageItem(QObject *parent = 0);
    ~ImageItem();

protected:
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);

signals:
    void currentPositionRgbChanged(QPointF&);

public slots:
};

#endif // IMAGEITEM_H
