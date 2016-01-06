#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QPixmap>
#include <QFileDialog>

#include <QMouseEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPointF>

#include <QPainter>

#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

#include <QDebug>

#include "cellcounter.h"

extern CellCounter Cells;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool eventFilter(QObject *obj, QEvent *event);
    //void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);
    void resizeEvent(QResizeEvent *);
    void updateImgLabel();

signals:
    void Mouse_Pos();
    void fileNameChanged(QString);

private slots:
    void on_actionLoad_Image_triggered();
    void on_thresholdValueSpin_valueChanged(int);
    void on_thresholdTypeBox_currentIndexChanged(int);
    void on_countCellsButton_clicked();
    void finishedCounting();

    void on_chooseCircleButton_clicked();
    void updateCircle();

    void on_minContourSizeSpin_valueChanged(int);
    void on_minPcaRatioSpin_valueChanged(double);
    void on_maxPcaRatioSpin_valueChanged(double);
    void on_minRadiusSpin_valueChanged(double arg1);
    void on_maxRadiusSpin_valueChanged(double arg1);

protected:

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene = new QGraphicsScene(this);
    QPixmap pixmapImg;
    QFutureWatcher<int> *watcher;
    //Need to get rid of these three bool variables, that allows to draw circle
    bool drawCircle = false; //gets false in updateImgLabel
    bool drawCircleAllowed = false; //gets false when button to accept circle was pressed
    bool updateCircleAllowed = false; //gets false in doUpdateCircle
    bool showColored = false;
    QPoint pixmapSize;
    QPoint mouseCurrentPos;
    QPoint circleCenter;
    int circleRadius = 150;
};

#endif // MAINWINDOW_H
