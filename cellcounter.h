#ifndef CELLCOUNTER_H
#define CELLCOUNTER_H

#include <QtGlobal>
#include <QString>
#include <QPixmap>
#include <QGraphicsScene>

#include <QDebug>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#include "defines.h"

#define BINARY 0
#define BINARY_INVERTED 1
#define TRUNCATE 2
#define TO_ZERO 3
#define TO_ZERO_INVERTED 4

#define BLACK 0x000000
#define WHITE 0xffffff


class CellCounter
{
private:
    cv::Mat imgOriginal, imgGray, img, imgColor, imgOccupied;
    QImage imgQ, imgQColor; //Saved for qt so we can just convert to Pixmap
    QString imgPath = "";
    int foundColonies = 0;
    cv::Point circleCenterPoint;
    int circleRadius;

    //User defined values
    int thresholdValue = 1;
    int thresholdType = BINARY_INVERTED;
    float minContourSize = 4.0;
    float minRadius = 0.1;
    float maxRadius = 10;
    float thresholdRadiusMin = 0.5;
    float thresholdRadiusMax = 0.5;
    int drawCircleSize = 5;
    float minCircleRatio = 1.0;
    float maxCircleRatio = 5.0;
    float scaleFactorCascade = 1.1;
    int minNeighborsCascade = 2;

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    std::vector<std::vector<cv::Point>> acceptedColonies;
    std::vector<cv::Point> points;

    std::vector<cv::Rect> singleColonies;

protected:

public:
    CellCounter();
    int loadImage(QString);
    void thresholdValueChanged(int);
    void thresholdTypeChanged(int);
    int countColoniesStandard(QPoint, int, QSize);
    void analyseBlobs(cv::Mat);
    bool isSpaceAlreadyOccupied(cv::Point, int);
    int countColoniesCascade(QPoint, int, QSize, QString);
    int analyseColoniesCascade(cv::CascadeClassifier);
    void calculateCircleCenterAndRadius(QPoint, int, QSize, cv::Mat);
    int isCircle(std::vector<cv::Point> &);
    std::vector<std::vector<cv::Point>> seperateColonies(std::vector<cv::Point> &, int);
    void addCircle(QPoint, QSize);

    void set_contourSize(int);
    void set_minRadius(double);
    void set_maxRadius(double);
    void set_minCircleRatio(double);
    void set_maxCircleRatio(double);
    void set_imgPath(QString);
    QString return_imgPath(void);
    QImage return_imgQOriginal(void);
    QImage return_imgQ(void);
    QImage return_imgQColored(void);
    int return_numberOfColonies(void);
};

#endif // CELLCOUNTER_H
