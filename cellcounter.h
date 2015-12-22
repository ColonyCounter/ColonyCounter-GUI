#ifndef CELLCOUNTER_H
#define CELLCOUNTER_H

#include <QtGlobal>
#include <QString>
#include <QPixmap>
#include <QGraphicsScene>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#define BINARY 0
#define BINARY_INVERTED 1
#define TRUNCATE 2
#define TO_ZERO 3
#define TO_ZERO_INVERTED 4

#define BLACK 0x000000
#define WHITE 0xffffff


class CellCounter
{
    cv::Mat imgOriginal, imgGray, img; //img is the one that changes
    QImage imgQOriginal, imgQ; //Saved for qt so we can just convert to Pixmap
    QString imgPath = "";
    int thresholdValue = 1;
    int thresholdType = BINARY_INVERTED;
    int maxPoints = 250;
    std::vector<cv::Point> points;

public:
    CellCounter();
    int loadImage(QString);
    void thresholdValueChanged(int);
    void thresholdTypeChanged(int);
    int countColonies(void);

    void set_imgPath(QString);
    QString return_imgPath(void);
    QImage return_imgQOriginal(void);
    QImage return_imgQ(void);
};

#endif // CELLCOUNTER_H
