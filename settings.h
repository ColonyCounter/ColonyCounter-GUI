#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>

namespace Ui {
class Settings;
}

class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = 0);
    ~Settings();

signals:
    double newSpValue(double);
    double newSrValue(double);
    void finished(void);

private slots:
    void on_buttonBox_accepted();

private:
    Ui::Settings *ui;
};

#endif // SETTINGS_H
