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
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

signals:
    void currentPositionRgbChanged(QPointF&);
    void pixelClicked(QPointF&);

public slots:
};

#endif // IMAGEITEM_H
