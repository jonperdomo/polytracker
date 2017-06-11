#include "framedisplay.h"
#include <QMessageBox>

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
    QPoint mouse_pos = mouse_event->pos();

    if(mouse_pos.x() <= this->size().width() && mouse_pos.y() <= this->size().height())
    {
        if(mouse_pos.x() >= 0 && mouse_pos.y() >= 0)
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
