#include "settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);
}

Settings::~Settings()
{
    delete ui;
}

void Settings::on_buttonBox_accepted()
{
    emit this->newSpValue(ui->SpatialSpin->value());
    emit this->newSrValue(ui->ColorSpin->value());
    emit this->finished();

    this->close();
}
