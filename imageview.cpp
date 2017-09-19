#include "imageview.h"

ImageView::ImageView(QWidget* parent) :
    band(QRubberBand::Rectangle, this),
    selecting(false),
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
    selecting = true;
    QGraphicsView::mousePressEvent(event);
    viewport()->setCursor(Qt::CrossCursor);
    origin = event->pos();
    //selection = QRect(origin, QSize());
    band.setGeometry(QRect(origin, QSize()));
    band.show();
    //qDebug() << "View point: " << origin.x() << ", " << origin.y();
    QPoint pixel = mapToScene(origin).toPoint();
    //qDebug() << "Pixel point: " << (int)pixel.x() << ", " << (int)pixel.y();
    selection = QRect(pixel, QSize());
    //qDebug() << "[Pressed] BL: " << (int)selection.bottomLeft().x() << ", " << (int)selection.bottomLeft().y() << " TR: " << (int)selection.topRight().x() << ", " << (int)selection.topRight().y();
    //emit pixelUpdate(pixel);
    emit selectionUpdate(selection);
}

void ImageView::mouseMoveEvent(QMouseEvent *event)
{
    QPoint endpoint = event->pos();
    //selection = QRect(origin, event->pos()).normalized();
    band.setGeometry(QRect(origin, endpoint).normalized());
    if (selecting)
    {
        QPoint BL = mapToScene(origin).toPoint();
        QPoint TR = mapToScene(endpoint).toPoint();
        selection = QRect(BL, TR).normalized();
        //qDebug() << "[Moved] BL: " << (int)selection.bottomLeft().x() << ", " << (int)selection.bottomLeft().y() << " TR: " << (int)selection.topRight().x() << ", " << (int)selection.topRight().y();
        //emit selectionUpdate(selection);
    }
}

void ImageView::mouseReleaseEvent(QMouseEvent *event)
{
    selecting = false;
    QGraphicsView::mouseReleaseEvent(event);
    viewport()->setCursor(Qt::CrossCursor);
    band.hide();
    emit selectionUpdate(selection);
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
