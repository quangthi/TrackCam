#include "mainwindow.h"
#include "ui_mainwindow.h"



bool    gIsVideoShown = false;
QTimer  *gSaveTimer, *gSendTimer;
CvSize  gVideoSize;
// Position for sending
short int	gCenterX = 0;
short int	gCenterY = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setFixedSize(this->width(),this->height());
    this->ui->radioMouse->setChecked(false);
    this->ui->radioJoystick->setChecked(true);    
    this->ui->EditWidth->setValidator( new QIntValidator(30, m_Config._config.frmWidth*0.8f, this));
    this->ui->EditHeight->setValidator( new QIntValidator(30, m_Config._config.frmHeight*0.8f, this));
    this->ui->FpsEdit->setValidator( new QIntValidator(18, 30, this));

    InitNetwork();

    createTrayActions();
    createTrayIcon();
    setTrayIcon();
    trayIcon->show();

    frmView = NULL;

    this->ui->EditWidth->setText(QString::number(m_Config._config.trkWidth));
    this->ui->EditHeight->setText(QString::number(m_Config._config.trkHeight));
    this->ui->FpsEdit->setText(QString::number(m_Config._config.fps));

    // Init saving timer
    gSaveTimer = new QTimer();
    connect(gSaveTimer, SIGNAL(timeout()), this, SLOT(OnSavingVideo()));
    // Init sending timer
    gSendTimer = new QTimer();
    connect(gSendTimer, SIGNAL(timeout()), this, SLOT(OnTimerSend()));

    gVideoSize = cvSize(m_Config._config.frmWidth, m_Config._config.frmHeight);
}

MainWindow::~MainWindow()
{
    if (m_CamUdpSocket)
    {
        m_CamUdpSocket->close();
        delete m_CamUdpSocket;
        m_CamUdpSocket = NULL;
    }

    EndSaving();
    gSaveTimer->stop();
    gSendTimer->stop();
    m_Config._config.trkWidth = this->ui->EditWidth->text().toDouble();
    m_Config._config.trkHeight = this->ui->EditHeight->text().toDouble();
    m_Config._config.fps = this->ui->FpsEdit->text().toDouble();
    m_Config.SaveToFile();
    delete ui;
    delete trayIcon;
    delete trayIconMenu;
    delete showHideTray;
    delete closeTray;
}

int inline ConvStrChar(std::string szStr, char *szBuff)
{
    int	nLen = szStr.length();

    nLen = (nLen < 254)? nLen : 254;		// Get min(nLeng,nSize)	- 254 = max buff
    for (int i = 0; i < nLen; i ++)
        szBuff[i] = (char)szStr[i];

    szBuff[nLen] = 0x00;
    return nLen;
}

void MainWindow::StartCam()
{
    if (frmView)
        return;

    if (!this->ui->radioMouse->isChecked())
    {
        if (((this->ui->EditWidth->text()).toDouble() < 30)||((this->ui->EditHeight->text()).toDouble() < 30)
                ||((this->ui->EditWidth->text()).toDouble() > m_Config._config.frmWidth*0.8f)
                ||((this->ui->EditHeight->text()).toDouble() >= m_Config._config.frmWidth*0.8f))
        {
            QMessageBox msgBox;
            msgBox.setText("Invalid tracking rect!");
            msgBox.exec();
            return;

        }
    }


    frmView = new VideoDisplay();
    frmView->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    frmView->show();
    gIsVideoShown = true;

    frmView->m_IsMouseOn = this->ui->radioMouse->isChecked();
    if (!frmView->m_IsMouseOn)
    {
        frmView->m_rectWidthInit  = (this->ui->EditWidth->text()).toDouble();
        frmView->m_rectHeightInit = (this->ui->EditHeight->text()).toDouble();
    }

    this->ui->radioMouse->setEnabled(false);
    this->ui->radioJoystick->setEnabled(false);
    this->ui->ShowHide->setText("Hide");

    gSendTimer->start(100);

}

void MainWindow::StopCam()
{
    if (!frmView)
        return;

    EndSaving();
    gSaveTimer->stop();
    gSendTimer->stop();
    this->ui->chkRec->setChecked(false);

    frmView->StopTracking();
    frmView->close();
    delete frmView;
    this->ui->radioMouse->setEnabled(true);
    this->ui->radioJoystick->setEnabled(true);

    frmView = NULL;
}


void MainWindow::on_Start_clicked()
{
    StartCam();
}

void MainWindow::on_Stop_clicked()
{

    StopCam();
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    if (trayIcon->isVisible())
    {
        showHideWindow();
        event->ignore();
    }

//    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "TrackCam",
//                                                                tr("Are you sure?\n"),
//                                                                QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
//                                                                QMessageBox::Yes);
//    if (resBtn != QMessageBox::Yes)
//    {
//        event->ignore();
//    }
//    else
//    {
//        if (frmView)
//            StopCam();
//        event->accept();
//    }
}


void MainWindow::on_btnTrack_clicked()
{
    if (!frmView)
        return;

    if (frmView->m_IsMouseOn)
        return;

    if (frmView->m_IsTracking)
        return;

    RECT inputRECT;

    inputRECT.left		= m_Config._config.frmWidth / 2 - frmView->m_rectWidthInit / 2;
    inputRECT.top		= m_Config._config.frmHeight / 2 - frmView->m_rectHeightInit / 2;
    inputRECT.right		= m_Config._config.frmWidth / 2 + frmView->m_rectWidthInit / 2;
    inputRECT.bottom	= m_Config._config.frmHeight / 2 + frmView->m_rectHeightInit / 2;

    frmView->StartTracking(inputRECT);
}

void MainWindow::on_btnRelease_clicked()
{
    if (!frmView)
        return;

//    if (frmView->m_IsMouseOn)
//        return;

    frmView->StopTracking();
}

void MainWindow::on_ShowHide_clicked()
{
    if (!frmView)
        return;

    if (gIsVideoShown)
    {
        frmView->hide();
        this->ui->ShowHide->setText("Show");
        gIsVideoShown = false;
    }
    else
    {
        frmView->show();
        this->ui->ShowHide->setText("Hide");
        gIsVideoShown = true;
    }


}

void MainWindow::on_cbtnIncrease_clicked()
{
    if (!frmView)
        return;
    if (frmView->m_IsMouseOn)
        return;
    if (frmView->m_IsTracking)
        return;
    if (frmView->m_rectWidthInit >= m_Config._config.frmWidth*0.8f)
        return;
    if (frmView->m_rectHeightInit >= m_Config._config.frmWidth*0.8f)
        return;

    frmView->m_rectWidthInit = frmView->m_rectWidthInit*1.1f;
    frmView->m_rectHeightInit = frmView->m_rectHeightInit*1.1f;

    this->ui->EditWidth->setText(QString::number(frmView->m_rectWidthInit));
    this->ui->EditHeight->setText(QString::number(frmView->m_rectHeightInit));
}

void MainWindow::on_cbtnDecrease_clicked()
{
    if (!frmView)
        return;
    if (frmView->m_IsMouseOn)
        return;
    if (frmView->m_IsTracking)
        return;
    if (frmView->m_rectWidthInit <= 30)
        return;
    if (frmView->m_rectHeightInit <= 30)
        return;
    frmView->m_rectWidthInit = frmView->m_rectWidthInit / 1.1f + 1;
    frmView->m_rectHeightInit = frmView->m_rectHeightInit / 1.1f + 1;

    this->ui->EditWidth->setText(QString::number(frmView->m_rectWidthInit));
    this->ui->EditHeight->setText(QString::number(frmView->m_rectHeightInit));
}

void MainWindow::OnSavingVideo()
{
    EndSaving();
    BeginSaving();
}

void MainWindow::BeginSaving()
{
    char		szTmp[255];
    int nCodec = CV_FOURCC('D', 'I', 'V', 'X');

    if (!QDir("Video").exists())
        QDir().mkdir("Video");

    std::string strFileName = GetStrTime().toStdString();

    strFileName = "./Video/" + strFileName + ".avi";

    ConvStrChar(strFileName, szTmp);

    if (frmView)
        frmView->m_Writer = cvCreateVideoWriter(szTmp, nCodec, this->ui->FpsEdit->text().toDouble(), gVideoSize);

}

void MainWindow::EndSaving()
{
    if (frmView)
    {
        cvReleaseVideoWriter(&frmView->m_Writer);
        frmView->m_Writer = NULL;
    }
}

bool MainWindow::CreateRecordTimerPeriod()
{
    int CurrInx = ui->cbSavePeriod->currentIndex();

    switch (CurrInx)
        {
        case 0:
            gSaveTimer->start(15000 * 60);
            break;
        case 1:
            gSaveTimer->start(30000 * 60);
            break;
        case 2:
            gSaveTimer->start(45000 * 60);
            break;
        case 3:
            gSaveTimer->start(60000 * 60);
            break;

        default:
            return false;
        }
    return true;
}

void MainWindow::on_chkRec_clicked(bool checked)
{
    if (!frmView)
    {
        ui->chkRec->setChecked(false);
        return;
    }

    if (checked)
    {
        if (((this->ui->FpsEdit->text()).toDouble() < 16)||((this->ui->FpsEdit->text()).toDouble() > 30))
        {
            QMessageBox msgBox;
            msgBox.setText("Invalid FPS (FPS = 16 - 30!");
            msgBox.exec();
            ui->chkRec->setChecked(false);
            return;
        }
        if(CreateRecordTimerPeriod())
            BeginSaving();
        else
        {
            QMessageBox msgBox;
            msgBox.setText("Cannot create timer for saving video!");
            msgBox.exec();
            ui->chkRec->setChecked(false);
            return;
        }
    }
    else
    {
        EndSaving();
        gSaveTimer->stop();
    }
}

QString MainWindow::GetStrTime()
{
    const QDateTime currTime = QDateTime::currentDateTime();
    const QString timeStamp = currTime.toString(QLatin1String("yyyyMMdd-hhmmss"));
    return timeStamp;
}

void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if(event->type() == QEvent::WindowStateChange)
    {
        if(isMinimized())
        this->hide();
    }
}


void MainWindow::showHideWindow()
{
    if(this->isVisible())
    {
        this->hide();
        showHideTray->setIcon(QIcon("./Icon/Show.ico"));
        showHideTray->setText("Show TrackCam Window");
    }
    else
    {
        this->show();
        showHideTray->setIcon(QIcon("./Icon/Hide.ico"));
        showHideTray->setText("Hide TrackCam Window");
    }
}

void MainWindow::trayIconClicked(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::DoubleClick)
        showHideWindow();
}

void MainWindow::setTrayIcon()
{
    trayIcon->setIcon(QIcon("./Icon/TrackCam.ico"));
}

void MainWindow::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(showHideTray);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(closeTray);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconClicked(QSystemTrayIcon::ActivationReason)));
}

void MainWindow::createTrayActions()
{
    showHideTray = new QAction(tr("&Hide TrackCam Window"), this);
    connect(showHideTray, SIGNAL(triggered()), this, SLOT(showHideWindow()));
    showHideTray->setIcon(QIcon("./Icon/Hide.ico"));
    closeTray = new QAction(tr("&Exit"), this);
    connect(closeTray, SIGNAL(triggered()), this, SLOT(on_btnExit_clicked()));
    closeTray->setIcon(QIcon("./Icon/Exit.ico"));
}


void MainWindow::on_btnExit_clicked()
{
        QMessageBox::StandardButton resBtn = QMessageBox::question( this, "TrackCam",
                                                                    tr("Are you sure?\n"),
                                                                    QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                                    QMessageBox::Yes);
        if (resBtn == QMessageBox::Yes)
        {
            if (frmView)
                StopCam();
            QApplication::quit();
        }


}

void MainWindow::InitNetwork()
{
    m_CamUdpSocket = new QUdpSocket(this);
    m_CamUdpSocket->bind(LOCAL_UDP_PORT);
    connect(m_CamUdpSocket, SIGNAL(readyRead()), this, SLOT(ProcMsgControl()));
}

void MainWindow::ProcMsgControl()
{
    if (!frmView)
        return;

    while (m_CamUdpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(m_CamUdpSocket->pendingDatagramSize());
        m_CamUdpSocket->readDatagram(datagram.data(), datagram.size());

        if (datagram.at(0) != 0xFF)
            break;

        if (datagram.at(1) == 0x01)         // start tracking
            on_btnTrack_clicked();
        else if (datagram.at(1) == 0x00)    // stop tracking
            on_btnRelease_clicked();
        else if (datagram.at(1) == 0x03)    // increase tracking rectangle
            on_cbtnIncrease_clicked();
        else if (datagram.at(1) == 0x02)    // decrease tracking rectangle
            on_cbtnDecrease_clicked();

        break;
    }

    return;
}

void MainWindow::SendMsgTrkPos(short int nX, short int nY)
{
    QByteArray datagram;
    datagram.resize(5);

    // Header
    datagram[0] = 0xFF;

    // X
    datagram[1] = BYTE(nX >> 8);
    datagram[2] = BYTE(nX );

    // Y
    datagram[3] = BYTE(nY >> 8);
    datagram[4] = BYTE(nY	   );

    m_CamUdpSocket->writeDatagram(datagram, QHostAddress::LocalHost, REMOTE_UDP_PORT);

}

void MainWindow::OnTimerSend()
{
    if (!frmView)
        return;
    if (!frmView->m_IsTracking)
        return;

    CvRect	nCvRectBox = cvRect(0, 0, 0, 0);
    utl_ConvertRectToBox(frmView->m_rectCurrent, &nCvRectBox);
    gCenterX = ((nCvRectBox.x + nCvRectBox.width / 2) - (m_Config._config.frmWidth / 2)) * 100 / m_Config._config.frmWidth;
    gCenterY = ((nCvRectBox.y + nCvRectBox.height / 2) - (m_Config._config.frmHeight / 2)) * 100 / m_Config._config.frmHeight;

    SendMsgTrkPos(gCenterX, gCenterY);
}
