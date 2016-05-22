#include "settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);

    // Load the current settings from the ColonyCounter object
    //Colonies
    ui->contourSizeSpin->setValue((double) Colonies.minContourSize);
    ui->minRadiusSpin->setValue((double) Colonies.minRadius);
    ui->maxRadiusSpin->setValue((double) Colonies.maxRadius);

    //Threshold
    ui->thresholdValueSpin->setValue((int) Colonies.thresholdValue);
    ui->thresholdTypeBox->setCurrentIndex((int) Colonies.thresholdType);

    //Mean Shift Filtering
    ui->enableMeanShiftCheckbox->setChecked(Colonies.pyrMeanShiftEnabled);
    ui->spatialSpin->setValue((double) Colonies.sp);
    ui->colorSpin->setValue((double) Colonies.sr);

    //PCA Analysis
    ui->minCircleRatioSpin->setValue((double) Colonies.minCircleRatio);
    ui->maxCircleRatioSpin->setValue((double) Colonies.maxCircleRatio);

    //Cascade Classifier
    ui->scaleFactorSpin->setValue((double) Colonies.scaleFactorCascade);
    ui->minNeighborsSpin->setValue((double) Colonies.minNeighborsCascade);
}

Settings::~Settings()
{
    delete ui;
}

void Settings::on_buttonBox_accepted()
{
    //Save all values
    //Colonies
    Colonies.minContourSize = (float) ui->contourSizeSpin->value();
    Colonies.minRadius = (float) ui->minRadiusSpin->value();
    Colonies.maxRadius = (float) ui->maxRadiusSpin->value();

    //Threshold
    Colonies.thresholdValue = (int) ui->thresholdValueSpin->value();
    Colonies.thresholdType = (int) ui->thresholdTypeBox->currentIndex();
    Colonies.thresholdTypeChanged(Colonies.thresholdType);

    //Mean Shift Filtering
    Colonies.pyrMeanShiftEnabled = (bool) ui->enableMeanShiftCheckbox->isChecked();
    Colonies.sp = (double) ui->spatialSpin->value();
    Colonies.sr = (double) ui->colorSpin->value();

    //PCA Analysis
    Colonies.minCircleRatio = (float) ui->minCircleRatioSpin->value();
    Colonies.maxCircleRatio = (float) ui->maxCircleRatioSpin->value();

    //Cascade Classifier
    Colonies.scaleFactorCascade = (float) ui->scaleFactorSpin->value();
    Colonies.minNeighborsCascade = (float) ui->minNeighborsSpin->value();

    emit this->finished();
    this->close();
}
