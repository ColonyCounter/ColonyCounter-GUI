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
        Colonies.set_imgPath(fileName);
        Colonies.loadImage(fileName);

        this->showColored = true;

        ui->countColoniesLabel->setText("Found colonies: 0");

        updateImgLabel();
    }
    return;
}

void MainWindow::updateImgLabel()
{
    //Convert imgQ to Pixmap and display in label
    if(!this->showColored) {
        this->pixmapImg = QPixmap::fromImage(Colonies.return_imgQ(), 0);
    }
    else if(this->showColored) {
        this->pixmapImg = QPixmap::fromImage(Colonies.return_imgQColored(), 0);
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

void MainWindow::on_countColoniesButton_clicked()
{
    //Disable add/delete of colonies
    this->editColonies = false;

    //Check which module/function for colony counting should be used
    if( this->useCascadeClassifier == true ) {
        qDebug() << "Use cascade classifier for counting.";

        //Start new thread to run countCells function, otherwise GUI freezes
        this->watcher = new QFutureWatcher<int>;
        connect(this->watcher, SIGNAL(finished()), this, SLOT(finishedCounting()));
        this->watcher->setFuture(QtConcurrent::run(&Colonies, &ColonyCounter::countColoniesCascade, this->circleCenter, this->circleRadius, this->pixmapSize, this->cascadeClassifierType));
    }
    else {
        qDebug() << "Use standard module for counting.";

        //Start new thread to run countCells function, otherwise GUI freezes
        this->watcher = new QFutureWatcher<int>;
        connect(this->watcher, SIGNAL(finished()), this, SLOT(finishedCounting()));
        this->watcher->setFuture(QtConcurrent::run(&Colonies, &ColonyCounter::countColoniesStandard, this->circleCenter, this->circleRadius, this->pixmapSize, this->activeModule));
    }

    //Add check: if return value is < 0 -> there was an error: check qDebug() and display Error message
    return;
}

void MainWindow::on_chooseCircleButton_clicked()
{
    this->drawCircleAllowed = true;

    ui->countColoniesLabel->setText("Found colonies: 0");

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
    int colonies = Colonies.return_numberOfColonies();
    ui->countColoniesLabel->setText(QString("Found colonies: %1").arg(colonies));
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
            this->circleRadius += 5;
        }
        else if( wheelDegrees.y() < 0 && this->circleRadius > 5) {
            this->circleRadius -= 5;
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

    if(event->type() == QEvent::MouseMove) {

        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        statusBar()->showMessage(QString("Mouse move (%1,%2)").arg(mouseEvent->pos().x()).arg(mouseEvent->pos().y()));

        if( !(this->drawCircle) && this->drawCircleAllowed) {
            //drawCircle indicates that a new circle should be drawn, position or radius changed
            //drawCircleAllowed is true after chooseCircle button was clicked and till user does right click
            //updateCircleAllowed reduces the amount of how many times the circle is redrawn, max every 0.1 seconds

            if( obj->objectName() == "imgLabel" ) {
                this->mouseCurrentPos.setX(mouseEvent->pos().x());
                this->mouseCurrentPos.setY(mouseEvent->pos().y());
                this->circleCenter = this->mouseCurrentPos;

                this->drawCircle = this->updateCircleAllowed = true;
                QTimer::singleShot(100, this, SLOT(updateCircle()));
            }
        }

    }

    if( event->type() == QEvent::MouseButtonRelease ) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        if( obj->objectName() == "imgLabel" ) {
            qDebug() << "Mouse pressed: x: " << mouseEvent->pos().x() << " y: " << mouseEvent->pos().y();

            QPoint mousePosition;
            mousePosition.setX(mouseEvent->pos().x());
            mousePosition.setY(mouseEvent->pos().y());

            if((mouseEvent->button() == Qt::LeftButton) && this->editColonies) {
                qDebug() << "Left mouse button pressed";

                Colonies.addCircle(mousePosition, this->pixmapSize);
                this->updateFoundColoniesStr(); //update string of found colonies
            }
            else if((mouseEvent->button() == Qt::RightButton) && this->editColonies) {
                qDebug() << "Right mouse button pressed";
                Colonies.removeCircle(mousePosition, this->pixmapSize);

                Colonies.drawCircles();
                this->updateFoundColoniesStr(); //update string of found colonies
            }
            else if((mouseEvent->button() == Qt::LeftButton) && this->drawCircleAllowed) {
                qDebug() << "left mouse pressed.";
                this->drawCircleAllowed = false;
            }
        }

    }


  return false;
}

void MainWindow::updateCircle()
{
    this->updateCircleAllowed = false;
    updateImgLabel();
}

void MainWindow::on_actionDefault_triggered()
{
    this->useCascadeClassifier = true;
    this->activeModule = cascade;

    this->cascadeClassifierType = DEFAULT_CASCADE;
    ui->moduleUsedLabel->setText(DEFAULT_CASCADE);

    this->showColored = true;

    Colonies.resetCounting();

    this->updateImgLabel();
}

void MainWindow::on_actionStandard_module_triggered()
{
    this->useCascadeClassifier = false;
    this->activeModule = standard;
    ui->moduleUsedLabel->setText("Standard");

    this->showColored = true;

    Colonies.resetCounting();

    this->updateImgLabel();
}

void MainWindow::on_actionSingle_colonies_triggered()
{
    this->useCascadeClassifier = false;
    this->activeModule = single;
    ui->moduleUsedLabel->setText("Single colonies");

    this->showColored = true;

    Colonies.resetCounting();

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

    this->updateImgLabel();
}

void MainWindow::on_actionSettings_triggered()
{
    this->settingsDialog = new Settings(this);

    connect(this->settingsDialog, SIGNAL(finished(void)), this, SLOT(finishedSettings()));

    this->settingsDialog->show();
}

void MainWindow::sp_valueChanged(double newValue)
{
    qDebug() << "New sp value: " << newValue;
    Colonies.spChanged(newValue);

    return;
}

void MainWindow::sr_valueChanged(double newValue)
{
    qDebug() << "New sr value: " << newValue;
    Colonies.srChanged(newValue);

    return;
}

void MainWindow::finishedSettings(void)
{
    if(Colonies.thresholdType == NONE) {
        this->showColored = true;
    }
    else {
        this->showColored = false;
    }

    updateImgLabel();
    qDebug() << "Finished settings";

    return;
}
