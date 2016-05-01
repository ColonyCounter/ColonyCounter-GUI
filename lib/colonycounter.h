#ifndef COLONYCOUNTER_H
#define COLONYCOUNTER_H

#include <QtGlobal>
#include <QString>
#include <QPixmap>
#include <QGraphicsScene>

#include <QDebug>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

//Name of the organism(s) and their corresponding .xml file(s)
#define DEFAULT_CASCADE "Default"
#define DEFAULT_XML "data/cascade.xml" //currently it is not E.Coli that we have trained with

#define BINARY 0
#define BINARY_INVERTED 1
#define TRUNCATE 2
#define TO_ZERO 3
#define TO_ZERO_INVERTED 4
#define NONE 5

#define MAX_ITER_TERM_CRITERIA 1000
#define EPS_TERM_CRITERIA 0.1

#define BLACK 0x000000
#define WHITE 0xffffff

struct point_radius {
    cv::Point pnt;
    float radius;
};


enum analyseModule {
    standard,
    single,
    watershed,
    cascade
};

typedef enum analyseModule analyseModule;

typedef struct point_radius point_radius;


class ColonyCounter
{
public:
    //User defined values
    int thresholdValue = 255;
    int thresholdType = NONE;

    //for pyrMeanShiftFiltering
    bool pyrMeanShiftEnabled = true;
    double sp = 20.0; //sp – The spatial window radius.
    double sr = 50.0; //sr – The color window radius.

    float minContourSize = 4.0;
    float minRadius = 0.5;
    float maxRadius = 10;

    float minCircleRatio = 1.0;
    float maxCircleRatio = 5.0;
    float scaleFactorCascade = 1.05;
    int minNeighborsCascade = 3;

private:
    cv::Mat imgOriginal, imgGray, img, imgColorOriginal, imgColor, imgOccupied;
    QImage imgQ, imgQColor; //Saved for qt so we can just convert to Pixmap
    QString imgPath = "";
    int foundColonies = 0;
    cv::Point circleCenterPoint;
    int circleRadius;

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    std::vector<std::vector<cv::Point>> acceptedColonies;
    std::vector<point_radius> acceptedMeansColonies;
    std::vector<cv::Point> points;

    std::vector<cv::Rect> cascadeColonies;

protected:

public:
    ColonyCounter();
    int loadImage(QString);
    void thresholdValueChanged(int);
    void thresholdTypeChanged(int);
    void spChanged(double);
    void srChanged(double);
    void make_pyrMeanShiftFiltering(void);
    void resetCounting(void);

    int countColoniesStandard(QPoint, int, QSize, analyseModule);
    void analyseBlobs(cv::Mat);
    void analyseBlobsAlternative(cv::Mat);
    void analyseBlobsWatershed(cv::Mat);
    void analyseContours(cv::Mat imgRoiColor);
    bool isSpaceAlreadyOccupied(cv::Point, int);
    int countColoniesCascade(QPoint, int, QSize, QString);
    int analyseColoniesCascade(cv::CascadeClassifier);

    void calculateCircleCenterAndRadius(QPoint, int, QSize, cv::Mat);
    int isCircle(std::vector<cv::Point> &);
    std::vector<std::vector<cv::Point>> seperateColonies(std::vector<cv::Point> &, int);
    void storeAndPaintColony(cv::Point, float, std::vector<cv::Point>);
    unsigned int root(unsigned int);
    void drawCircles();
    void addCircle(QPoint, QSize);
    void removeCircle(QPoint, QSize);

    void set_imgPath(QString);
    QString return_imgPath(void);
    QImage return_imgQOriginal(void);
    QImage return_imgQ(void);
    QImage return_imgQColored(void);
    int return_numberOfColonies(void);
};

#endif // COLONYCOUNTER_H
