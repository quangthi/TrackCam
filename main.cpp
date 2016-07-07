#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;


    //w.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    w.setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);

    // Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint

    w.show();

    return a.exec();
}
