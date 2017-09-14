#include "imageview.h"

ImageView::ImageView(QWidget* parent) :
    band(QRubberBand::Rectangle, this),
    QGraphicsView(parent)
{

}

ImageView::~ImageView()
{

}

void ImageView::enterEvent(QEvent *event)
{
    QGraphicsView::enterEvent(event);
    viewport()->setCursor(Qt::CrossCursor);
}

void ImageView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
    viewport()->setCursor(Qt::CrossCursor);
    origin = event->pos();
    band.setGeometry(QRect(origin, QSize()));
    band.show();
}

void ImageView::mouseMoveEvent(QMouseEvent *event)
{
    QPoint drag_point = event->pos();
    band.setGeometry(QRect(origin, event->pos()).normalized());
}

void ImageView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    viewport()->setCursor(Qt::CrossCursor);
    band.hide();
}

void ImageView::wheelEvent(QWheelEvent *event)
{
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    /// Scale the view / do the zoom
    double scaleFactor = 1.15;
    if(event->delta() > 0) {
        /// Zoom in
        scale(scaleFactor, scaleFactor);

    } else {
        /// Zooming out
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }
}
