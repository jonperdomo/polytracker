#include "imageitem.h"

#include <QDebug>

ImageItem::ImageItem(QObject *parent)
{
    // NOTE: May not need to track mouse movement at all times.
    this->setAcceptHoverEvents(true);
}

ImageItem::~ImageItem()
{

}

void ImageItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QPointF mouse_pos = event->pos();
    QRgb rgbValue = pixmap().toImage().pixel(mouse_pos.x(), mouse_pos.y());
    qDebug() << "x: " << mouse_pos.x() << ", y: " << mouse_pos.y();
    emit currentPositionRgbChanged(mouse_pos);
}
