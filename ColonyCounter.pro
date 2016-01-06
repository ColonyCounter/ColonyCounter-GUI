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

PKGCONFIG += opencv

INCLUDEPATH += /usr/local/include/opencv
LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_imgproc

SOURCES += main.cpp\
        mainwindow.cpp \
    cellcounter.cpp

HEADERS  += mainwindow.h \
    cellcounter.h

FORMS    += mainwindow.ui

DISTFILES += \
    to-do.txt \
    to-do.txt
