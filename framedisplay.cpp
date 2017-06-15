#include "framedisplay.h"
#include <QMessageBox>
#include <QPainter>

FrameDisplay::FrameDisplay(QWidget *parent) : QLabel(parent)
{
    // NOTE: May not need to track mouse movement at all times.
    this->setMouseTracking(true);
}

FrameDisplay::~FrameDisplay()
{

}

void FrameDisplay::mouseMoveEvent(QMouseEvent *mouse_event)
{
    if (this->pixmap() > 0)
    {
        QPoint mouse_pos = mouse_event->pos();

        // Convert to Pixmap coordinates
        int width_spacer = this->size().width() - this->pixmap()->width();
        int height_spacer = this->size().height() - this->pixmap()->height();
        int x = mouse_pos.x() - (this->size().width() - this->pixmap()->width()) / 2;
        int y = mouse_pos.y() - (this->size().height() - this->pixmap()->height()) / 2;
        mouse_pos.setX(x);
        mouse_pos.setY(y);
        if (x >= 0 && x <= this->pixmap()->width() && y >= 0 &&  y <= this->pixmap()->height())
        {
            emit sendMousePosition(mouse_pos);
        }
    }
}

void FrameDisplay::mousePressEvent(QMouseEvent *mouse_event)
{
    QMessageBox msg;
    if(mouse_event->button() == Qt::LeftButton)
    {
        msg.setText("Left Mouse Button pressed!");
        msg.exec();
    }
    else if (mouse_event->button() == Qt::RightButton)
    {
        msg.setText("Right Mouse Button pressed!");
        msg.exec();
    }
}

void FrameDisplay::paintEvent(QPaintEvent *paint_event)
{
    QPainter p(this);
    p.drawEllipse(this->x(), this->y(), width() /10, height() /10);
    p.end();
    QLabel::paintEvent(paint_event);
}
