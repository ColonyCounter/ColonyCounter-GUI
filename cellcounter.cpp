#include "cellcounter.h"

CellCounter::CellCounter()
{

}

int CellCounter::loadImage(QString fileName)
{
    if( fileName.isEmpty() ) {
        qWarning("f: Opening image was cancelled.");
        return 0;
    }

    imgOriginal = cv::imread(fileName.toStdString().c_str(), CV_LOAD_IMAGE_COLOR);

    if( imgOriginal.empty() ) {
        qWarning("f: Image is empty.");
        return 0;
    }

    //cv::cvtColor(img, img, CV_BGR2RGB); //Qt uses RGB and opencv BGR
    cv::cvtColor(imgOriginal, imgGray, CV_BGR2GRAY);

    imgQOriginal = QImage((uchar*) imgOriginal.data, imgOriginal.cols, imgOriginal.rows, imgOriginal.step, QImage::Format_RGB888); //not needed right know and false conversion (FORMAT)

    return 1;
}

void CellCounter::thresholdValueChanged(int thresholdValueArg)
{
    thresholdValue = thresholdValueArg;
    cv::threshold(imgGray, img, thresholdValue, 255, thresholdType);

    imgQ = QImage((uchar*) img.data, img.cols, img.rows, img.step, QImage::Format_Indexed8);
    return;
}

void CellCounter::thresholdTypeChanged(int thresholdTypeArg)
{
    thresholdType = thresholdTypeArg;
    cv::threshold(imgGray, img, thresholdValue, 255, thresholdType);

    imgQ = QImage((uchar*) img.data, img.cols, img.rows, img.step, QImage::Format_Indexed8);
    return;
}

int CellCounter::countColonies(void)
{
    qDebug("Starting counting colonies.");

    //Threshold to Binary Inverted to get white blobs
    //And change to user defined threshold value
    cv::Mat imgProcessed;
    cv::threshold(imgGray, imgProcessed, thresholdValue, 255, thresholdType); // Let the user change between Binary and Binary Inverted

    int currentPoints = 0; //number of found colonies
    int sum = 0;
    for(int i = 0; i<10000; i++) {
        sum *= i;
    }

    int maxY = imgProcessed.rows;
    int maxX = imgProcessed.cols;

    //Just allow a max number of points
    //Improve code -> use smaller picture let user choose

    for(int y=0; y<maxY; y++) {
        qDebug("New row!");
        for(int x=0; x<maxX; x++) {
            qDebug("X: %d\tY: %d", x, y);
            int pixelColor = imgProcessed.at<uchar>(cv::Point(x, y));
            if( pixelColor == 255 ) { //it's white
                cv::Rect rect;
                //Fill the white blob/colony
                cv::floodFill(imgProcessed, cv::Point(x,y), CV_RGB(255,0,0), &rect, cv::Scalar(0), cv::Scalar(0), 4);
                currentPoints++;
                    //check colony if good add to vector of points and increase currentPoints
            }//color-if
        }//x-for
    }//y-for

    //Just for simplicity for testing
    imgQ = QImage((uchar*) imgProcessed.data, imgProcessed.cols, imgProcessed.rows, imgProcessed.step, QImage::Format_Indexed8);
    qDebug("%d", currentPoints);
    return currentPoints;
}

void CellCounter::set_imgPath(QString fileName)
{
    this->imgPath = fileName;
}

QString CellCounter::return_imgPath(void)
{
    return this->imgPath;
}

QImage CellCounter::return_imgQOriginal(void)
{
    return this->imgQOriginal;
}

QImage CellCounter::return_imgQ(void)
{
    return this->imgQ;
}
