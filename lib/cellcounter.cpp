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

    this->imgOriginal = cv::imread(fileName.toStdString().c_str(), CV_LOAD_IMAGE_COLOR);

    if( this->imgOriginal.empty() || !(this->imgOriginal.data) ) {
        qWarning("f: Image is empty.");
        return 0;
    }

    cv::cvtColor(this->imgOriginal, this->imgOriginal, CV_RGB2BGR); //Qt uses RGB and opencv BGR
    cv::cvtColor(imgOriginal, imgGray, CV_BGR2GRAY); //imgGray should not be changes, it's the original image but gray

    return 1;
}

void CellCounter::thresholdValueChanged(int thresholdValueArg)
{
    //Save value and update image
    this->thresholdValue = thresholdValueArg;
    cv::equalizeHist( this->imgGray, this->imgGray );

    cv::threshold(imgGray, img, thresholdValue, 255, thresholdType);

    imgQ = QImage((uchar*) img.data, img.cols, img.rows, img.step, QImage::Format_Indexed8);
    return;
}

void CellCounter::thresholdTypeChanged(int thresholdTypeArg)
{
    //Save type and update image
    this->thresholdType = thresholdTypeArg;
    cv::equalizeHist( this->imgGray, this->imgGray );
    cv::threshold(imgGray, img, thresholdValue, 255, thresholdType);

    imgQ = QImage((uchar*) img.data, img.cols, img.rows, img.step, QImage::Format_Indexed8);
    return;
}

int CellCounter::countColoniesStandard(QPoint circleCenter, int circleRad, QSize pixmapSize, analyseModule activeModule)
{
    qDebug() << "Starting counting colonies on: " << this->return_imgPath();

    //Reset variables for a new start
    this->acceptedColonies.clear();
    this->acceptedMeansColonies.clear();
    this->contours.clear();
    this->hierarchy.clear();
    this->foundColonies = 0;

    this->calculateCircleCenterAndRadius(circleCenter, circleRad, pixmapSize, this->img);

    //BINARY_INVERTED if colonies are darker than background, BINARY otherwise
    //Change img to BINARY_INVERTED
    if(this->thresholdType == BINARY_INVERTED) {
        cv::threshold(this->imgGray, this->img, this->thresholdValue, 255, BINARY_INVERTED);
    }
    else {
        //use BINARY if user defined so
        cv::threshold(this->imgGray, this->img, this->thresholdValue, 255, BINARY);
    }

    //Create mask of petri dish
    cv::Mat mask = cv::Mat::zeros(this->img.rows, this->img.cols, CV_8UC1);
    cv::circle(mask, this->circleCenterPoint, this->circleRadius, cv::Scalar(255, 255, 255), -1); //-1 means circle is filled, lineType=8 and shift= 0 << standard values
    cv::Mat imgRoiTemp;
    this->img.copyTo(imgRoiTemp, mask);

    //Create Region of Interest, reduzed size
    cv::Mat imgRoi(imgRoiTemp, cv::Rect(this->circleCenterPoint.x-this->circleRadius, this->circleCenterPoint.y-this->circleRadius, this->circleRadius*2, this->circleRadius*2));

    //Reset image to use isSpaceAlreadyOccupied()
    this->imgOccupied = cv::Mat(imgRoi.rows, imgRoi.cols, CV_8UC1, cv::Scalar(0, 0, 0));

    if( activeModule == single ) {
        this->analyseBlobsAlternative(imgRoi);
    }
    else {
        this->analyseBlobs(imgRoi);
    }

    return 0;
}

void CellCounter::analyseBlobs(cv::Mat imgRoi)
{
    cv::Mat imgRoiColor;
    cv::cvtColor(imgRoi, imgRoiColor, CV_GRAY2BGR);

    //store the original roi colored, to redraw circle if needed
    imgRoiColor.copyTo(this->imgColorOriginal);

    //int edgeThresh = 1;
    int cannyThreshold = 100; //no real differences between threshold values, because of just two colors (black and white)
    int ratio = 3;
    int kernelSize = 3;

    cv::Mat cannyOutput;

    //Detect edges using canny
    imgRoi.copyTo(cannyOutput);
    cv::Canny(cannyOutput, cannyOutput, cannyThreshold, cannyThreshold*ratio, kernelSize);

    cv::findContours(cannyOutput, this->contours, this->hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    analyseContours(imgRoiColor);

    imgRoiColor.copyTo(this->imgColor);

    return;
}

void CellCounter::analyseBlobsAlternative(cv::Mat imgRoi)
{
    cv::Mat imgRoiColor;
    cv::cvtColor(imgRoi, imgRoiColor, CV_GRAY2BGR);

    //store the original roi colored, to redraw circle if needed
    imgRoiColor.copyTo(this->imgColorOriginal);

    //Extract edges with laplacian
    cv::Mat tempImgRoi;
    imgRoi.copyTo(tempImgRoi);

    cv::Laplacian(imgRoi, imgRoi, 0, 5);


    //dilate
    cv::dilate(imgRoi, imgRoi, cv::Mat());

    //subtract images
    cv::Mat imgRoiDiff = imgRoi - tempImgRoi;

    //get contours
    //cv::findContours(imgRoiDiff.clone(), this->contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
    cv::findContours(imgRoiDiff, this->contours, this->hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    /*double minArea = this->minRadius;
    double minCircleRatio = 0.1;
    for(unsigned int i=0; i<this->contours.size(); ++i)
    {
        double cArea = cv::contourArea(this->contours[i]);
        if(cArea < minArea) continue;

        //filteredContours.push_back(contours[i]);
        //cv::drawContours(input, contours, i, cv::Scalar(0,255,0), 1);
        cv::Point2f center;
        float radius;
        cv::minEnclosingCircle(this->contours[i], center, radius);

        double circleArea = radius*radius*CV_PI;

        if(cArea/circleArea < minCircleRatio) continue;

        //cv::circle(imgRoiColor, center, radius, cv::Scalar(0,0,255),2);
    }*/
    this->analyseContours(imgRoiColor);

    imgRoiColor.copyTo(this->imgColor);
    cv::imwrite("2_imgRoiColor.jpg", imgRoiColor);

    return;
}

void CellCounter::analyseContours(cv::Mat imgRoiColor)
{
    for(std::vector<cv::Point> contour: this->contours) {
        //calculate the mean/center point
        cv::Point sumPoint = std::accumulate(contour.begin(), contour.end(), cv::Point(0, 0));
        cv::Point meanPoint = sumPoint * (1.0 / contour.size());

        //calculate the mean radius
        unsigned int iCounter = 0;
        float meanRadius = 0;
        float x = 0;
        float y = 0;
        for(iCounter=0; iCounter < contour.size(); iCounter++) {
            x = (float) contour.at(iCounter).x;
            y = (float) contour.at(iCounter).y;

            //calculate distance between meanPoint and current processed point
            float distanceX = (float) (x-meanPoint.x)*(x-meanPoint.x);
            float distanceY = (float) (y-meanPoint.y)*(y-meanPoint.y);
            meanRadius += (float) sqrt(distanceX+distanceY);
            //maybe replace sqrt with: https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Babylonian_method
            //or use integer for meanRadius, then this->root can be used
        }

        meanRadius = meanRadius / contour.size(); //get the mean of it
        float circleArea = (float) M_PI * meanRadius * meanRadius;
        if( (!(circleArea > 0)) || (circleArea < this->minRadius) ) {
            //circle area cannot be zero but radius can be set zero by user, Next
            continue;
        }

        qDebug() << "Circle Area: "<< circleArea;

        int cSize = contour.size();
        if(cSize < this->minContourSize) { //should be at least a rectangle I think, c_size=4 (default value)
            continue; //next one, not enough edges
        }

        //First consider if the blob is made up of more than one colony
        //Needs improvements!
        int k = 0;
        for(float i=this->maxRadius; i>= this->minRadius; i -= 0.1) {
            float iCircleArea = (float) M_PI * i * i;
            //double area = cv::contourArea(contour); //not precise enough

            int n = (int) circleArea / iCircleArea; //round it
            if(n > 0) {
                k = n;
                break;
            }
        }

        if(k == 0) {
            continue;
        }

        else if(k == 1) {
            if( !(this->isCircle(contour)) || (this->minRadius > meanRadius) || (this->maxRadius < meanRadius) ) {
                //Not a colony
                continue;
            }

            if( this->isSpaceAlreadyOccupied(meanPoint, meanRadius) ) {
                continue; //There is already something, next one
            }

            //Store center point to vector
            point_radius tempPointRadius;
            tempPointRadius.pnt = meanPoint;
            tempPointRadius.radius = meanRadius;

            //Accept found colony
            this->acceptedColonies.push_back(contour);
            this->acceptedMeansColonies.push_back(tempPointRadius);

            //Paint it
            cv::circle(imgRoiColor, meanPoint, meanRadius, cv::Scalar(255, 0, 0), 2, 8);

            this->foundColonies++;
        }

        else if(k > 1) {
            qDebug() << "Splitting, k = " << k;
            std::vector<std::vector<cv::Point>> tempColonies = this->seperateColonies(contour, k);
            for(std::vector<cv::Point> tempColony: tempColonies) {
                cv::Point sum = std::accumulate(tempColony.begin(), tempColony.end(), cv::Point(0, 0));
                cv::Point mean = sum * (1.0 / tempColony.size());

                //Get the mean radius
                meanRadius = 0;
                for(cv::Point pnt: tempColony) {
                    float distanceX = (float) (pnt.x-mean.x)*(pnt.x-mean.x);
                    float distanceY = (float) (pnt.y-mean.y)*(pnt.y-mean.y);
                    meanRadius += (float) sqrt(distanceX+distanceY);
                }
                meanRadius /= tempColony.size();

                if(!(this->isCircle(tempColony)) || (this->minRadius > meanRadius) || (this->maxRadius < meanRadius)) {
                    //Not a colony, next
                    continue;
                }

                if( this->isSpaceAlreadyOccupied(meanPoint, meanRadius) ) {
                    continue; //Already, next one
                }

                point_radius tempPntRadius;
                tempPntRadius.pnt = mean;
                tempPntRadius.radius = meanRadius;

                this->acceptedColonies.push_back(tempColony);
                this->acceptedMeansColonies.push_back(tempPntRadius);

                //Paint it
                cv::circle(imgRoiColor, mean, meanRadius, cv::Scalar(255, 0, 0), 2, 8);
            }
        }

    }
}

bool CellCounter::isSpaceAlreadyOccupied(cv::Point meanPoint, int meanRadius)
{
    //check if already a colony is nearby -> probably same colony
    //check not all pixels, just a few along the radius and different angles
    std::vector<cv::Point2f> pointsToCheck(8, meanPoint);
    float rad = (2 * meanRadius)/3;
    pointsToCheck.at(0).x += rad;
    pointsToCheck.at(1).x -= rad;
    pointsToCheck.at(2).y += rad;
    pointsToCheck.at(3).y += rad;
    pointsToCheck.at(4).x += rad; pointsToCheck.at(4).y += rad;
    pointsToCheck.at(5).x += rad; pointsToCheck.at(5).y -= rad;
    pointsToCheck.at(6).x -= rad; pointsToCheck.at(6).y += rad;
    pointsToCheck.at(7).x -= rad; pointsToCheck.at(7).y -= rad;

    for(cv::Point2f pnt: pointsToCheck) {
        cv::Scalar color = this->imgOccupied.at<uchar>(pnt);
        if( color.val[0] == 255 ) {
            qDebug() << pnt.x << "," << pnt.y << "already found.";
            return true; //there is already one
        }
    }

    //no other colony at same palce, add it to Mat
    cv::circle(this->imgOccupied, meanPoint, meanRadius, cv::Scalar(255, 255, 255), -1, 8);

    //if not add circle with mean radius to Mat
    return false;
}

int CellCounter::countColoniesCascade(QPoint circleCenter, int circleRad, QSize pixmapSize, QString organism)
{
    qDebug() << "Using cascade";
    qDebug() << "Organism" << organism;
    qDebug() << "Starting counting colonies on: " << this->return_imgPath();

    this->calculateCircleCenterAndRadius(circleCenter, circleRad, pixmapSize, this->img);

    //Load the right .xml files based on user choice
    std::string singleColonyXML;
    if( organism.compare(E_COLI) == 0 ) {
        singleColonyXML = E_COLI_XML;
    }

    cv::CascadeClassifier singleColonyCascade;
    if( !singleColonyCascade.load(singleColonyXML) ) {
        qCritical() << "Could not load .xml file(s).";
        return -1;
    }

    int foundColonies = 0;
    foundColonies += this->analyseColoniesCascade(singleColonyCascade);

    return foundColonies;
}

int CellCounter::analyseColoniesCascade(cv::CascadeClassifier singleColonyCascade)
{
    int foundColonies = 0;
    // Reset the vectors to start again
    this->singleColonies.clear();

    cv::Mat imgGray, imgResult;
    this->img.copyTo(imgResult);

    cv::cvtColor(this->img, imgGray, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(imgGray, imgGray);

    //Detect single colonies
    //LBP classifer used right know, maybe change to Haar
    singleColonyCascade.detectMultiScale(imgGray, this->singleColonies, this->scaleFactorCascade, this->minNeighborsCascade,
                                  0, cv::Size(this->minRadius, this->minRadius), cv::Size(this->maxRadius, this->maxRadius));

    for(cv::Rect colony: this->singleColonies) {
        cv::Point center(colony.x + colony.width/2, colony.y + colony.height/2);
        cv::circle(imgResult, center, (colony.width+colony.height)/2, cv::Scalar(255, 0, 0), 2, 8);
    }
    foundColonies += this->singleColonies.size();

    //Add the detection of more than one colony

    imgResult.copyTo(this->imgColor);

    return foundColonies;
}

void CellCounter::calculateCircleCenterAndRadius(QPoint circleCenter, int circleRad, QSize pixmapSize, cv::Mat imgOrig)
{
    //Recalculate factor for radius and circleCenter
    float factorWidth = (float) imgOrig.cols / pixmapSize.width();
    float factorHeight = (float) imgOrig.rows / pixmapSize.height();

    //convert QPoint to OpenCV point
    this->circleCenterPoint.x = circleCenter.x() * factorWidth;
    this->circleCenterPoint.y = circleCenter.y() * factorHeight;

    this->circleRadius = circleRad;
    if(factorWidth > factorHeight) // maybe swap to avoid accessing Mat elements that do not exist
        this->circleRadius *= factorWidth;
    else
        this->circleRadius *= factorWidth;

    return;
}

int CellCounter::isCircle(std::vector<cv::Point> &data)
{
    cv::Mat dataBuffer = cv::Mat(data.size(), 2, CV_32F);
    for(int i = 0; i < dataBuffer.rows; i++) {
        dataBuffer.at<float>(i, 0) = data[i].x;
        dataBuffer.at<float>(i, 1) = data[i].y;
    }

    cv::PCA pcaAnalysis(dataBuffer, cv::Mat(), CV_PCA_DATA_AS_ROW);

    //Calculate the ratio
    float ratio = sqrt(pcaAnalysis.eigenvalues.at<float>(0)) / sqrt(pcaAnalysis.eigenvalues.at<float>(1));

    if(ratio <= this->minCircleRatio || ratio > this->maxCircleRatio) {
        return 0;
        qDebug() << "No circle";
    }

    return 1;
}

std::vector<std::vector<cv::Point>> CellCounter::seperateColonies(std::vector<cv::Point> &data, int k)
{
    //Set k-means criteria
    cv::TermCriteria kCriteria = cv::TermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 1000, 0.1);

    cv::Mat dataBuffer = cv::Mat(data.size(), 2, CV_32F);
    for(int i=0; i <dataBuffer.rows; i++) {
        dataBuffer.at<float>(i, 0) = data[i].x;
        dataBuffer.at<float>(i, 1) = data[i].y;
    }

    cv::Mat centers, labels;
    cv::kmeans(dataBuffer, k, labels, kCriteria, 1, cv::KMEANS_RANDOM_CENTERS, centers);

    //Create the vector<vector<Point>>
    std::vector<std::vector<cv::Point>> returnVector;
    for(int i =0; i < k; i++) {
        std::vector<cv::Point> sub; //stores one of the centers

        for(int j=0; j<labels.rows; j++) {
            if(labels.at<int>(j, 0) == i) {
                sub.push_back(data[j]);
            }
        }

        returnVector.push_back(sub);
    }

    return returnVector;
}

unsigned int root(unsigned int x)
{
    //Source: http://supp.iar.com/FilesPublic/SUPPORT/000419/AN-G-002.pdf
    unsigned int a,b;
    b = x;
    a = x = 0x3f;
    x = b/x;
    a = x = (x+a)>>1;
    x = b/x;
    a = x = (x+a)>>1;
    x = b/x;
    x = (x+a)>>1;

    return(x);
}

void CellCounter::drawCircles()
{
    this->imgColorOriginal.copyTo(this->imgColor);

    for(point_radius pntAndRad: this->acceptedMeansColonies) {
        cv::circle(this->imgColor, pntAndRad.pnt, pntAndRad.radius, cv::Scalar(255, 0, 0), 2, 8);
    }

    return;
}

void CellCounter::addCircle(QPoint cursorPoint, QSize pixmapSize)
{
    int radius = (this->minRadius+this->maxRadius)/2;
    this->calculateCircleCenterAndRadius(cursorPoint, radius, pixmapSize, this->imgColor);

    //Add point to vector with just the center
    cv::Point pnt(this->circleCenterPoint.x, this->circleCenterPoint.y);
    std::vector<cv::Point> pntVector = {pnt};
    this->acceptedColonies.push_back(pntVector);

    point_radius tempPointRadius;
    tempPointRadius.pnt = pnt;
    tempPointRadius.radius = radius;
    this->acceptedMeansColonies.push_back(tempPointRadius);

    cv::circle(this->imgColor, this->circleCenterPoint, radius, cv::Scalar(255, 0, 0), 2, 8);

    return;
}

void CellCounter::removeCircle(QPoint cursorPoint, QSize pixmapSize)
{
    int nearestPoint = -1;
    this->calculateCircleCenterAndRadius(cursorPoint, this->maxRadius, pixmapSize, this->imgColor);

    //Search for nearest circle to remove
    int vectorSize = this->acceptedMeansColonies.size();

    if( vectorSize < 1 ) {
        return; //no elements in vector
    }

    int i = 0;
    cv::Point pnt = this->acceptedMeansColonies.at(i).pnt - this->circleCenterPoint;
    unsigned int oldDistance = pnt.x * pnt.x + pnt.y * pnt.y; //initial point to compare to
    for(i=0; i < vectorSize; i++) {
        //Only Checks the first of the saved points to speed this up
        pnt = this->acceptedMeansColonies.at(i).pnt - this->circleCenterPoint;

        unsigned int newDistance = pnt.x * pnt.x + pnt.y * pnt.y;
        if( newDistance < oldDistance ) {
            oldDistance = newDistance;
            nearestPoint = i;
        }
    }

    //Delete the found colony from the vector
    if( nearestPoint > -1 ) {
        this->acceptedColonies.erase(this->acceptedColonies.begin()+nearestPoint);
        this->acceptedMeansColonies.erase(this->acceptedMeansColonies.begin()+nearestPoint);
    }
}

void CellCounter::set_contourSize(int newSize)
{
    this->minContourSize = newSize;
}

void CellCounter::set_minRadius(double newRadius)
{
    this->minRadius = (float) newRadius;
}

void CellCounter::set_maxRadius(double newRadius)
{
    this->maxRadius = (float) newRadius;
}

void CellCounter::set_minCircleRatio(double newRatio)
{
    this->minCircleRatio = (float) newRatio;
}

void CellCounter::set_maxCircleRatio(double newRatio)
{
    this->maxCircleRatio = (float) newRatio;
}

void CellCounter::set_imgPath(QString fileName)
{
    this->imgPath = fileName;
}

QString CellCounter::return_imgPath(void)
{
    return this->imgPath;
}

QImage CellCounter::return_imgQ(void)
{
    return this->imgQ;
}

QImage CellCounter::return_imgQColored(void)
{
    //cv::cvtColor(this->imgColor, this->imgColor, CV_RGB2BGR);//Qt uses RGB and opencv BGR but conversion was already done
    this->imgQColor = QImage((uchar*) imgColor.data, imgColor.cols, imgColor.rows, imgColor.step, QImage::Format_RGB888);
    return this->imgQColor;
}

int CellCounter::return_numberOfColonies(void)
{
    return this->acceptedColonies.size();
}
