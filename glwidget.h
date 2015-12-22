#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QObject>
#include <QGLWidget>

#include "mainwindow.h"

class GLWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit GLWidget(QWidget *parent = 0);
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);

private slots:
    void showImage(QString);
    void enableCircleDraw(void);
    void doUpdateCircle(void);

protected:
    QImage data, gldata, backgroundimage;

private:
    int mousePosX = 0;
    int mousePosY = 0;
    int circleRadius = 100;
    bool mouseTrackingEnabled = false;
    bool drawCircle = false;
    bool updateCircleAllowed = false;
};

#endif // GLWIDGET_H
