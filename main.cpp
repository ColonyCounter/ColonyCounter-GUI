#include "mainwindow.h"
#include <QApplication>
#include <QFileDialog>

ColonyCounter Colonies;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
