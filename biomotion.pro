#-------------------------------------------------
#
# Project created by QtCreator 2017-06-03T21:47:55
#
#-------------------------------------------------

QT += core gui \
    charts
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = biomotion
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    main.cpp \
    mainwindow.cpp \
    imageitem.cpp \
    imageview.cpp \
    chart.cpp

HEADERS += \
    mainwindow.h \
    imageitem.h \
    imageview.h \
    chart.h

FORMS += \
        mainwindow.ui

INCLUDEPATH += C:\Builds\OpenCV\opencv-VS141\install\include

CONFIG(debug,debug|release) {
LIBS += -LC:\Builds\OpenCV\opencv-VS141\install\x64\vc15\lib
LIBS += -lopencv_core320d
LIBS += -lopencv_imgproc320d
LIBS += -lopencv_highgui320d
LIBS += -lopencv_ml320d
LIBS += -lopencv_video320d
LIBS += -lopencv_videoio320d
LIBS += -lopencv_features2d320d
LIBS += -lopencv_calib3d320d
LIBS += -lopencv_objdetect320d
LIBS += -lopencv_flann320d
LIBS += -lopencv_imgcodecs320d
}
CONFIG(release,debug|release) {
LIBS += -LC:\Builds\OpenCV\opencv-VS141\install\x64\vc15\lib
LIBS += -lopencv_core320
LIBS += -lopencv_imgproc320
LIBS += -lopencv_highgui320
LIBS += -lopencv_ml320
LIBS += -lopencv_video320
LIBS += -lopencv_videoio320
LIBS += -lopencv_features2d320
LIBS += -lopencv_calib3d320
LIBS += -lopencv_objdetect320
LIBS += -lopencv_flann320
LIBS += -lopencv_imgcodecs320
}

RESOURCES += \
    images.qrc
