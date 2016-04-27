#-------------------------------------------------
#
# Project created by QtCreator 2015-11-15T16:30:56
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ColonyCounter
TEMPLATE = app

CONFIG += c++11

DEFINES += PI_CAM=false

PKGCONFIG += opencv

INCLUDEPATH += /usr/local/include/opencv
LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_objdetect

SOURCES += main.cpp\
    mainwindow.cpp \
    picam.cpp \
    settings.cpp \
    lib/colonycounter.cpp

HEADERS  += mainwindow.h \
    defines.h \
    picam.h \
    settings.h \
    lib/colonycounter.h

FORMS    += mainwindow.ui \
    settings.ui

DISTFILES += \
    to-do.txt \
    LICENSE \
    README.md
