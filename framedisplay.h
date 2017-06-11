#ifndef FRAMEDISPLAY_H
#define FRAMEDISPLAY_H

#include <QObject>
#include <QWidget>
#include <QLabel>

#include <QMouseEvent>

class FrameDisplay : public QLabel
{
    Q_OBJECT

public:
    FrameDisplay(QWidget *parent = 0);
    ~FrameDisplay();

protected:
    void mouseMoveEvent(QMouseEvent *mouse_event);
    void mousePressEvent(QMouseEvent *mouse_event);

signals:
    void sendMousePosition(QPoint&);

};

#endif // FRAMEDISPLAY_H
