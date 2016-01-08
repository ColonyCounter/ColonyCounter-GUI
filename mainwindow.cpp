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
        Cells.thresholdTypeChanged(1);
        Cells.thresholdValueChanged(70);

        this->showColored = false;
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

    pixmapImg = pixmapImg.scaled(w,h,Qt::KeepAspectRatio);
    //Pixmap width and height different to ui->imgLabel's
    this->pixmapSize.setWidth(pixmapImg.width());
    this->pixmapSize.setHeight(pixmapImg.height());


    if( this->drawCircle ) {
        QPainter painter(&(this->pixmapImg));
        QPen circlePen(Qt::red);
        circlePen.setWidth(2);

        painter.setPen(circlePen);
        qDebug() << mouseCurrentPos;
        painter.drawEllipse(mouseCurrentPos, circleRadius, circleRadius);

        this->drawCircle = false;
        qDebug() << "Drawed";
    }

    // set a scaled pixmap to a w x h window keeping its aspect ratio
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
    //Check which module/function for colony counting should be used
    if( this->useCascadeClassifier == true ) {
        qDebug() << "Use cascade classifier for counting.";
        this->update();
        //Start new thread to run countCells function, otherwise GUI freezes
        this->watcher = new QFutureWatcher<int>;
        connect(this->watcher, SIGNAL(finished()), this, SLOT(finishedCounting()));
        this->watcher->setFuture(QtConcurrent::run(&Cells, &CellCounter::countColoniesCascade, this->circleCenter, this->circleRadius, this->pixmapSize, this->cascadeClassifierType));
    }
    else {
        qDebug() << "Use standard module for counting.";
        this->update();
        //Start new thread to run countCells function, otherwise GUI freezes
        this->watcher = new QFutureWatcher<int>;
        connect(this->watcher, SIGNAL(finished()), this, SLOT(finishedCounting()));
        this->watcher->setFuture(QtConcurrent::run(&Cells, &CellCounter::countColoniesStandard, this->circleCenter, this->circleRadius, this->pixmapSize));
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
    int colonies = Cells.return_numberOfColonies();
    ui->countCellsLabel->setText(QString("Found colonies: %1").arg(colonies));
    this->showColored = true;
    this->updateImgLabel();

    //Call Destructor of QFutureWatcher
    this->watcher->~QFutureWatcher();

    return;
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if( this->drawCircleAllowed ) {
        this->drawCircleAllowed = false;
    }

    qDebug() << "Mouse pressed down.";
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    QPoint wheelDegrees = event->angleDelta()/8;
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
  if (event->type() == QEvent::MouseMove)
  {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
    statusBar()->showMessage(QString("Mouse move (%1,%2)").arg(mouseEvent->pos().x()).arg(mouseEvent->pos().y()));
    if( !drawCircle && drawCircleAllowed ) {
        this->mouseCurrentPos.setX(mouseEvent->pos().x());
        this->mouseCurrentPos.setY(mouseEvent->pos().y());
        this->circleCenter = this->mouseCurrentPos;

        this->drawCircle = this->updateCircleAllowed = true;
        QTimer::singleShot(100, this, SLOT(updateCircle()));
    }
  }

  return false;
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

void MainWindow::on_actionE_coli_triggered()
{
    this->useCascadeClassifier = true;
    this->cascadeClassifierType = E_COLI;
    ui->moduleUsedLabel->setText(E_COLI);
}

void MainWindow::on_actionStandard_module_triggered()
{
    this->useCascadeClassifier = false;
    ui->moduleUsedLabel->setText("Standard");
}
