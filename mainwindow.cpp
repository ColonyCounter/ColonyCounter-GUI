#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qApp->installEventFilter(this);

    setMouseTracking(true);
    //ui->imgLabel->setMouseTracking(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_loadImageButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select a file to open...", QDir::homePath());
    if( !fileName.isEmpty() ) {
        //Set standard configuration for loaded image
        Cells.set_imgPath(fileName);
        Cells.loadImage(fileName);
        Cells.thresholdTypeChanged(1);
        Cells.thresholdValueChanged(70);

        updateImgLabel();
    }
    return;
}

void MainWindow::updateImgLabel()
{
    //Convert imgQ to Pixmap and display in label
    pixmapImg = QPixmap::fromImage(Cells.return_imgQ(), 0);
    ui->imgLabel->setPixmap(pixmapImg);

    int w = ui->imgLabel->width();
    int h = ui->imgLabel->height();

    pixmapImg = pixmapImg.scaled(w,h,Qt::KeepAspectRatio);

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
   //QMainWindow::resizeEvent(event);
   updateImgLabel();

   qDebug() << "Resized";
}

void MainWindow::on_thresholdValueSpin_valueChanged(int thresholdValue)
{
    Cells.thresholdValueChanged(thresholdValue);
    updateImgLabel();

    return;
}

void MainWindow::on_thresholdTypeBox_currentIndexChanged(int index)
{
    Cells.thresholdTypeChanged(index);
    updateImgLabel();

    return;
}

void MainWindow::on_countCellsButton_clicked()
{
    // Call fill flood loop in Cells class
    // Update label
    //ui->countCellsLabel->setText("str");
    //Start new thread to run countCells function, otherwise GUI freezes
    QFutureWatcher<int> watcher;
    watcher.setFuture(QtConcurrent::run(&Cells, &CellCounter::countColonies));
    //QFuture<int> future = QtConcurrent::run(&Cells, &CellCounter::countColonies);

    //qDebug() << future.result();

}

void MainWindow::on_chooseCircleButton_clicked()
{
    this->drawCircleAllowed = true;
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
    if( this->drawCircleAllowed ) {

        QPoint wheelDegrees = event->angleDelta()/8;

        if( wheelDegrees.y() > 0 ) {
            this->circleRadius += 10;
        }
        else if( wheelDegrees.y() < 0 ) {
            this->circleRadius -= 10;
        }

        this->drawCircle = true;
        updateImgLabel();
    }

    qDebug() << "New CircleRadius: " << this->circleRadius;
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
