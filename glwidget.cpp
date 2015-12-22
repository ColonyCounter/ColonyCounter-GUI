#include "glwidget.h"

GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent)
{
    setMouseTracking(false);
    //connect(QTimer, SIGNAL(timeout()), this, SLOT(updateCircle()));
}

void GLWidget::initializeGL()
{
    QString imagePath = Cells.return_imgPath();

    if( imagePath.isEmpty() ) {
        //Otherwise window bugs on resize
        QImage stdImage(1, 1, QImage::Format_RGB16);
        data = stdImage;
    }
    else {
        data.load(imagePath);
    }

    gldata = QGLWidget::convertToGLFormat(data);
    resize(data.size());
    qDebug("InitializeGL");
}

void GLWidget::paintGL()
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDrawPixels(data.width(), data.height(), GL_RGBA, GL_UNSIGNED_BYTE, gldata.bits());


    if( updateCircleAllowed ) {
        int mousePosYInverse = 0;
        glColor3f(1, 0, 0);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        for(int i=0; i < 360; i += 20) {
            mousePosYInverse = (-1)*(mousePosY-this->height()); //otherwise circle goes up when mouse goes down
            float alpha = i*M_PI/180.0;
            float xPos = mousePosX + circleRadius*sin(alpha);
            float yPos = mousePosYInverse + circleRadius*cos(alpha);
            glVertex2f(xPos, yPos);
        }
        glEnd();
        this->updateCircleAllowed = false;
    }
    qDebug("paintGL");
}

void GLWidget::resizeGL(int w, int h)
{
    glViewport (0, 0, w, h);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();

    qDebug("resizeGL");
}

void GLWidget::showImage(QString fileName)
{
    //Display new Image

    QString imagePath = Cells.return_imgPath();

    data.load(imagePath);
    gldata = QGLWidget::convertToGLFormat(data);

    resize(data.size());

    updateGL(); //Problem: Not getting resized correctly, hiding buttons
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    //Save start point
    this->drawCircle = false;
    setMouseTracking(false);
    this->mouseTrackingEnabled = false;
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    //Change image position based on start position
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    //Move circle if circle button was pressed until right mouse click
    if(!drawCircle) {
        this->mousePosX = event->pos().x();
        this->mousePosY = event->pos().y();
        this->drawCircle = true;
        this->updateCircleAllowed = true;
        QTimer::singleShot(100, this, SLOT(doUpdateCircle()));
    }
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    QPoint wheelDegrees = event->angleDelta()/8;

    if( mouseTrackingEnabled ) {
        if( wheelDegrees.y() > 0 ) {
            this->circleRadius += 15;
        }
        else if( wheelDegrees.y() < 0 ) {
            this->circleRadius -= 15;
        }
        updateCircleAllowed = true;
        updateGL();
    }

    qDebug("Wheel");
}

void GLWidget::enableCircleDraw(void)
{
    //Only track mouse when we have to update the circle position
    setMouseTracking(true);
    this->mouseTrackingEnabled = true;
}

void GLWidget::doUpdateCircle(void)
{
    this->drawCircle = false;
    updateGL();
}
