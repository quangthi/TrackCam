#include "mainwindow.h"
#include <QApplication>
#include <QSharedMemory>

int main(int argc, char *argv[])
{
    /*
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    w.show();
    return a.exec();
    */

    QApplication a(argc, argv);
    a.processEvents();

    QSharedMemory shared("62d60669-bb94-4a94-88bb-b964890a7e04");

    if( !shared.create( 512, QSharedMemory::ReadWrite) )
    {
        QMessageBox msgBox;
        msgBox.setText( QObject::tr("Can't start more than one instance of TrackCam!") );
        msgBox.setIcon( QMessageBox::Critical );
        msgBox.exec();
        exit(0);
    }

    MainWindow w;
    w.setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    w.show();
    return a.exec();
}
