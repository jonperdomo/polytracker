#include "imageitem.h"



ImageItem::ImageItem(QObject *parent)
{

}

ImageItem::~ImageItem()
{

}

void ImageItem::mouseMoveEvent(QMouseEvent *mouse_event)
{
    QPoint mouse_pos = mouse_event->pos();
    emit currentPositionRgbChanged(mouse_pos, mouse_pos);
}
