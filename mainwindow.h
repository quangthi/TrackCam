#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <QCloseEvent>
#include <QDateTime>
#include <QUdpSocket>
#include <QHostAddress>
#include <QSystemTrayIcon>

#include "videodisplay.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    VideoDisplay    *frmView;
    //int				m_rectWidthInit;
    //int				m_rectHeightInit;
    CvVideoWriter*	m_Writer;

    CConfig         m_Config;
    QUdpSocket      *m_CamUdpSocket;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    void    StartCam();
    void    StopCam();
    void    BeginSaving();
    void    EndSaving();
    bool    CreateRecordTimerPeriod();
    QString GetStrTime();
    void    SendMsgTrkPos(short int, short int);

    void    InitNetwork();

    // Tray Icon Functions
    void createTrayActions();
    void createTrayIcon();
    void setTrayIcon();

    void closeEvent(QCloseEvent *);
    void changeEvent(QEvent *);

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QAction *showHideTray;
    QAction *closeTray;


//protected:
//    void closeEvent (QCloseEvent *event);

private slots:    
    void on_Start_clicked();
    void on_Stop_clicked();
    void on_btnTrack_clicked();
    void on_btnRelease_clicked();
    void on_ShowHide_clicked();    
    void on_cbtnIncrease_clicked();
    void on_cbtnDecrease_clicked();
    void OnSavingVideo();
    void on_chkRec_clicked(bool checked);

    void ProcMsgControl();

    // Tray Icon Functions
    void trayIconClicked(QSystemTrayIcon::ActivationReason);
    void showHideWindow();
    void on_btnExit_clicked();
    void OnTimerSend();
};

#endif // MAINWINDOW_H
