#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qApp->installEventFilter(this);

    setMouseTracking(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionLoad_Image_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select a file to open...", QDir::homePath());
    if( !fileName.isEmpty() ) {
        //Set standard configuration for loaded image
        Cells.set_imgPath(fileName);
        Cells.loadImage(fileName);
        Cells.thresholdTypeChanged(BINARY_INVERTED);
        Cells.thresholdValueChanged(THRESHOLD_VALUE);

        ui->countCellsLabel->setText("Found colonies: 0");
        updateImgLabel();
    }
    return;
}

void MainWindow::updateImgLabel()
{
    //Convert imgQ to Pixmap and display in label
    if(!this->showColored) {
        this->pixmapImg = QPixmap::fromImage(Cells.return_imgQ(), 0);
    }
    else if(this->showColored) {
        this->pixmapImg = QPixmap::fromImage(Cells.return_imgQColored(), 0);
    }

    int w  = ui->imgLabel->width();
    int h  = ui->imgLabel->height();

    this->pixmapImg = this->pixmapImg.scaled(w, h, Qt::KeepAspectRatio);
    //Pixmap width and height different to ui->imgLabel's
    this->pixmapSize.setWidth(pixmapImg.width());
    this->pixmapSize.setHeight(pixmapImg.height());


    if( this->drawCircle ) {
        //circle center not equal to mouse cursor as pixmap not occupying all of qlabel
        QPainter painter(&(this->pixmapImg));
        QPen circlePen(Qt::red);
        circlePen.setWidth(2);

        painter.setPen(circlePen);
        qDebug() << mouseCurrentPos;
        painter.drawEllipse(mouseCurrentPos, circleRadius, circleRadius);

        this->drawCircle = false;
        qDebug() << "Drawed";
    }

    ui->imgLabel->setPixmap(pixmapImg);
    qDebug() << "Updated";
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
   updateImgLabel();

   qDebug() << "Resized";
}

void MainWindow::on_thresholdValueSpin_valueChanged(int thresholdValue)
{
    Cells.thresholdValueChanged(thresholdValue);

    this->showColored = false;
    updateImgLabel();

    return;
}

void MainWindow::on_thresholdTypeBox_currentIndexChanged(int index)
{
    Cells.thresholdTypeChanged(index);

    this->showColored = false;
    updateImgLabel();

    return;
}

void MainWindow::on_countCellsButton_clicked()
{
    //Disable add/delete of colonies
    this->editColonies = false;

    //Check which module/function for colony counting should be used
    if( this->useCascadeClassifier == true ) {
        qDebug() << "Use cascade classifier for counting.";

        //Start new thread to run countCells function, otherwise GUI freezes
        this->watcher = new QFutureWatcher<int>;
        connect(this->watcher, SIGNAL(finished()), this, SLOT(finishedCounting()));
        this->watcher->setFuture(QtConcurrent::run(&Cells, &CellCounter::countColoniesCascade, this->circleCenter, this->circleRadius, this->pixmapSize, this->cascadeClassifierType));
    }
    else {
        qDebug() << "Use standard module for counting.";

        //Start new thread to run countCells function, otherwise GUI freezes
        this->watcher = new QFutureWatcher<int>;
        connect(this->watcher, SIGNAL(finished()), this, SLOT(finishedCounting()));
        this->watcher->setFuture(QtConcurrent::run(&Cells, &CellCounter::countColoniesStandard, this->circleCenter, this->circleRadius, this->pixmapSize, this->activeModule));
    }

    //Add check: if return value is < 0 -> there was an error: check qDebug() and display Error message
    return;
}

void MainWindow::on_chooseCircleButton_clicked()
{
    this->drawCircleAllowed = true;
    this->showColored = false;

    ui->countCellsLabel->setText("Found colonies: 0");
    this->update();
}

void MainWindow::finishedCounting()
{
    qDebug() << "Finished counting";

    this->updateFoundColoniesStr();

    //Call Destructor of QFutureWatcher
    this->watcher->~QFutureWatcher();

    return;
}

void MainWindow::updateFoundColoniesStr(void)
{
    qDebug() << "Update found colonies string";
    int colonies = Cells.return_numberOfColonies();
    ui->countCellsLabel->setText(QString("Found colonies: %1").arg(colonies));
    this->showColored = true;
    this->updateImgLabel();

    return;
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    QPoint wheelDegrees = event->angleDelta()/8;

    //Increase or decrease circle radius
    if( this->drawCircleAllowed ) {

        if( wheelDegrees.y() > 0) {
            this->circleRadius += 10;
        }
        else if( wheelDegrees.y() < 0 && this->circleRadius > 10) {
            this->circleRadius -= 10;
        }

        this->drawCircle = true;
        qDebug() << "New CircleRadius: " << this->circleRadius;
        updateImgLabel();
    }
}

//Could not get it to work, it needed a mouse click to trigger, although mouse tracking was set to true
//Event filter is used
/*void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if( this->drawCircle ) {
        this->mouseCurrentPos.setX(event->pos().x());
        this->mouseCurrentPos.setY(event->pos().y());

        this->updateImgLabel();
        qDebug() << "new mouse pos for circle";
    }
    qDebug() << event->pos();
}*/

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{

    if (event->type() == QEvent::MouseMove) {

    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
    statusBar()->showMessage(QString("Mouse move (%1,%2)").arg(mouseEvent->pos().x()).arg(mouseEvent->pos().y()));

    if( !this->drawCircle && this->drawCircleAllowed ) {
        //drawCircle indicates that a new circle should be drawn, position or radius changed
        //drawCircleAllowed is true after chooseCircle button was clicked and till user does right click
        //updateCircleAllowed reduces the amount of how many times the circle is redrawn, max every 0.1 seconds
        this->mouseCurrentPos.setX(mouseEvent->pos().x());
        this->mouseCurrentPos.setY(mouseEvent->pos().y());
        this->circleCenter = this->mouseCurrentPos;

        this->drawCircle = this->updateCircleAllowed = true;
        QTimer::singleShot(100, this, SLOT(updateCircle()));
    }

  }


  return false;
}

void MainWindow::mousePressEvent(QMouseEvent *mouseEvent)
{

    if( this->drawCircleAllowed && (mouseEvent->buttons() == Qt::LeftButton) ) {
        this->drawCircleAllowed = false;
          qDebug() << "Mouse pressed down.";
    }

    else if( this->editColonies ) {
        //Need to recalculate position, as image does not occupy all of qLabel
        //imgLabel width cannot be subtracted when window gets larger without resizing image
        //float x = (float) mouseEvent->pos().x() - (ui->imgLabel->pos().x() + ui->imgLabel->width() - this->pixmapSize.width());
        //float y = (float) mouseEvent->pos().y() - (ui->imgLabel->pos().y() + ui->imgLabel->height() - this->pixmapSize.height());
        float x = (float) mouseEvent->pos().x() - (ui->imgLabel->pos().x());
        float y = (float) mouseEvent->pos().y() - (ui->imgLabel->size().height() - this->pixmapSize.height()); //gets wrong when window gets resized

        qDebug() << "x: " << ui->imgLabel->pos().x() << "y: " << ui->imgLabel->pos().y();
        qDebug() << "pixmap.x: " << this->pixmapSize.width() << "pixmap.y: " << this->pixmapSize.height();
        qDebug() << "imgLabel.x: " << ui->imgLabel->size().width() << "imgLabel.y: " << ui->imgLabel->size().height();
        qDebug() << "imgLabelSize - pixmapSize" << (ui->imgLabel->size().height() - this->pixmapSize.height());

        QPoint calculatedPosition;
        calculatedPosition.setX((int) x);
        calculatedPosition.setY((int) y);

        if( mouseEvent->buttons() == Qt::LeftButton ) {
            qDebug() << "Left mouse button pressed" << mouseEvent->pos();
            qDebug() << ui->imgLabel->pos();
            Cells.addCircle(calculatedPosition, this->pixmapSize);
        }
        else if( mouseEvent->buttons() == Qt::RightButton ) {
            qDebug() << "Right mouse button pressed" << mouseEvent->pos();
            qDebug() << ui->imgLabel->pos();
            Cells.removeCircle(calculatedPosition, this->pixmapSize);

            Cells.drawCircles();
        }
        this->updateFoundColoniesStr(); //update string of found colonies
    }
}

void MainWindow::updateCircle()
{
    this->updateCircleAllowed = false;
    updateImgLabel();
}

void MainWindow::on_minContourSizeSpin_valueChanged(int newSize)
{
    Cells.set_contourSize(newSize);
}

void MainWindow::on_minRadiusSpin_valueChanged(double newValue)
{
    Cells.set_minRadius((float) newValue);
}

void MainWindow::on_maxRadiusSpin_valueChanged(double newValue)
{
    Cells.set_maxRadius((float) newValue);
}


void MainWindow::on_minPcaRatioSpin_valueChanged(double newValue)
{
    Cells.set_minCircleRatio((float) newValue);
}

void MainWindow::on_maxPcaRatioSpin_valueChanged(double newValue)
{
    Cells.set_maxCircleRatio((float) newValue);
}

void MainWindow::disableWidgets(void)
{
    ui->thresholdValueLabel->setEnabled(false);
    ui->thresholdValueSpin->setEnabled(false);
    ui->thresholdTypeLabel->setEnabled(false);
    ui->thresholdTypeBox->setEnabled(false);
    ui->minContourSizeLabel->setEnabled(false);
    ui->minContourSizeSpin->setEnabled(false);
    ui->minPcaRatioLabel->setEnabled(false);
    ui->minPcaRatioSpin->setEnabled(false);
    ui->maxPcaRatioLabel->setEnabled(false);
    ui->maxPcaRatioSpin->setEnabled(false);
    ui->minRadiusLabel->setEnabled(false);
    ui->minRadiusSpin->setEnabled(false);
    ui->maxRadiusLabel->setEnabled(false);
    ui->maxRadiusSpin->setEnabled(false);
}

void MainWindow::enableWidgets(void)
{
    ui->thresholdValueLabel->setEnabled(true);
    ui->thresholdValueSpin->setEnabled(true);
    ui->thresholdTypeLabel->setEnabled(true);
    ui->thresholdTypeBox->setEnabled(true);
    ui->minContourSizeLabel->setEnabled(true);
    ui->minContourSizeSpin->setEnabled(true);
    ui->minPcaRatioLabel->setEnabled(true);
    ui->minPcaRatioSpin->setEnabled(true);
    ui->maxPcaRatioLabel->setEnabled(true);
    ui->maxPcaRatioSpin->setEnabled(true);
    ui->minRadiusLabel->setEnabled(true);
    ui->minRadiusSpin->setEnabled(true);
    ui->maxRadiusLabel->setEnabled(true);
    ui->maxRadiusSpin->setEnabled(true);
}

void MainWindow::on_actionE_coli_triggered()
{
    this->useCascadeClassifier = true;
    this->activeModule = cascade;

    this->cascadeClassifierType = E_COLI;
    ui->moduleUsedLabel->setText(E_COLI);

    this->showColored = true;

    this->disableWidgets();
    this->updateImgLabel();
}

void MainWindow::on_actionStandard_module_triggered()
{
    this->useCascadeClassifier = false;
    this->activeModule = standard;
    ui->moduleUsedLabel->setText("Standard");

    this->showColored = false;

    this->enableWidgets();
    this->updateImgLabel();
}

void MainWindow::on_actionSingle_colonies_triggered()
{
    this->useCascadeClassifier = false;
    this->activeModule = single;
    ui->moduleUsedLabel->setText("Single colonies");

    this->showColored = false;

    this->enableWidgets();
    this->updateImgLabel();
}

void MainWindow::on_add_deleteColoniesButton_clicked()
{
    //Swap it
    this->editColonies = !(this->editColonies);
}

void MainWindow::on_actionWatershed_triggered()
{
    this->useCascadeClassifier = false;
    this->activeModule = watershed;
    ui->moduleUsedLabel->setText("Watershed");

    this->showColored = true;

    this->disableWidgets();
    this->updateImgLabel();
}

void MainWindow::on_actionSettings_triggered()
{
    this->settingsDialog = new Settings(this);

    connect(this->settingsDialog, SIGNAL(newSpValue(double)), this, SLOT(sp_valueChanged(double)));
    connect(this->settingsDialog, SIGNAL(newSrValue(double)), this, SLOT(sr_valueChanged(double)));
    connect(this->settingsDialog, SIGNAL(finished(void)), this, SLOT(finishedSettings()));

    this->settingsDialog->show();
}

void MainWindow::sp_valueChanged(double newValue)
{
    qDebug() << "New sp value: " << newValue;
    Cells.spChanged(newValue);

    return;
}

void MainWindow::sr_valueChanged(double newValue)
{
    qDebug() << "New sr value: " << newValue;
    Cells.srChanged(newValue);

    return;
}

void MainWindow::finishedSettings(void)
{
    Cells.make_pyrMeanShiftFiltering();
    updateImgLabel();

    qDebug() << "Finished settings";

    return;
}
