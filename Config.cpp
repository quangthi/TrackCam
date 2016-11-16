#include "Config.h"
#include "qdir.h"

CConfig::CConfig()
{
    //LoadConfigFile();
    LoadXmlFile();
}

void CConfig::LoadXmlFile()
{
    if (!QDir(CONF_PATH).exists())
        QDir().mkdir(CONF_PATH);

    CvFileStorage* fs = NULL;

    try
    {
        fs = cvOpenFileStorage(XML_FILE, 0, CV_STORAGE_READ);
    }
    catch (...)
    {
        setDefault();
        return;
    }


    if (fs == NULL)
    {
        setDefault();
        return;
    }

    _config.ipCam               = cvReadIntByName(fs, 0, "IpCam", 0);
    _config.strCamUrl_GV        = cvReadStringByName(fs, 0, "VsCamGV", CAM_URL_GV);
    _config.strCamUrl_GV_ir     = cvReadStringByName(fs, 0, "IrCamGV", CAM_URL_GV_IR);
    _config.strCamUrl_IP        = cvReadStringByName(fs, 0, "VsCamIP", CAM_URL_IP);
    _config.strCamUrl_IP_ir     = cvReadStringByName(fs, 0, "IrCamIP", CAM_URL_IP_IR);
    _config.trkWidth            = cvReadIntByName(fs, 0, "TrkWidth", QApplication::desktop()->screenGeometry(0).width()/6);
    _config.trkHeight           = cvReadIntByName(fs, 0, "TrkHeight", QApplication::desktop()->screenGeometry(0).height()/6);
    _config.frmPosX             = cvReadIntByName(fs, 0, "FrmPosX", 0);
    _config.frmPosY             = cvReadIntByName(fs, 0, "FrmPosY", 0);
    _config.frmWidth            = cvReadIntByName(fs, 0, "FrmWidth", QApplication::desktop()->screenGeometry(0).width());
    _config.frmHeight           = cvReadIntByName(fs, 0, "FrmHeight", QApplication::desktop()->screenGeometry(0).height());
    _config.fps                 = cvReadRealByName(fs, 0, "Fps", 18.5);


    cvReleaseFileStorage(&fs);
}

bool CConfig::LoadConfigFile()
{           
    if (!QDir(CONF_PATH).exists())
        QDir().mkdir(CONF_PATH);

    QFile file(CFG_FILE);
    if(!file.open(QIODevice::ReadOnly))
    {
        setDefault();
        return false;
    }
    QTextStream in(&file);
    QString line = in.readLine();           // camera url
    _config.strCamUrl_GV = line.toStdString();
    line = in.readLine();                   // ir camera url
    _config.strCamUrl_GV_ir = line.toStdString();
    line = in.readLine();                   // traking rectangle width
    _config.trkWidth = line.toDouble();
    line = in.readLine();                   // traking rectangle height
    _config.trkHeight = line.toDouble();
    line = in.readLine();                   // video frame position X
    _config.frmPosX = line.toDouble();
    line = in.readLine();                   // video frame position Y
    _config.frmPosY = line.toDouble();
    line = in.readLine();                   // video frame width
    _config.frmWidth = line.toDouble();
    line = in.readLine();                   // video frame height
    _config.frmHeight = line.toDouble();
    line = in.readLine();                   // frame per second to record
    _config.fps = line.toDouble();
    line = in.readLine();                   // get frame from ip cam or webcam
    _config.ipCam = line.toDouble();

    if(_config.strCamUrl_GV.size()<1)
    {
        _config.strCamUrl_GV = CAM_URL_GV;
    }

    if(_config.strCamUrl_GV_ir.size()<1)
    {
        _config.strCamUrl_GV = CAM_URL_GV_IR;
    }

    file.close();
    return true;
}


void CConfig::setDefault()
{    

    // Opt1: non-FullScreen
    _config.frmWidth        = QApplication::desktop()->screenGeometry(0).width()/2;
    _config.frmHeight       = QApplication::desktop()->screenGeometry(0).height()/2;
    _config.frmPosX         = QApplication::desktop()->screenGeometry(0).width()/2;
    _config.frmPosY         = QApplication::desktop()->screenGeometry(0).height()/2;
    //Opt2: FullScreen
//    _config.frmWidth        = QApplication::desktop()->screenGeometry(0).width();
//    _config.frmHeight       = QApplication::desktop()->screenGeometry(0).height();
//    _config.frmPosX         = 0;
//    _config.frmPosY         = 0;

    _config.trkWidth        = _config.frmWidth/6;
    _config.trkHeight       = _config.frmHeight/6;
    _config.strCamUrl_GV    = CAM_URL_GV;
    _config.strCamUrl_GV_ir = CAM_URL_GV_IR;
    _config.strCamUrl_IP    = CAM_URL_IP;
    _config.strCamUrl_IP_ir = CAM_URL_IP_IR;
    _config.fps             = FPS;
    _config.ipCam           = 1;

    SaveXmlFile();    
}

void CConfig::SaveToFile()
{
    QFile configFile(CFG_FILE);
    if(!configFile.open(QIODevice::WriteOnly))return;

    QTextStream outStream(&configFile);

    outStream<<_config.strCamUrl_GV.data()<<"\n";
    outStream<<_config.strCamUrl_GV_ir.data()<<"\n";
    outStream<<QString::number(_config.trkWidth)<<"\n";
    outStream<<QString::number(_config.trkHeight)<<"\n";
    outStream<<QString::number(_config.frmPosX)<<"\n";
    outStream<<QString::number(_config.frmPosY)<<"\n";
    outStream<<QString::number(_config.frmWidth)<<"\n";
    outStream<<QString::number(_config.frmHeight)<<"\n";
    outStream<<QString::number(_config.fps)<<"\n";
    outStream<<QString::number(_config.ipCam)<<"\n";
    //Close the file
    configFile.close();
}

void CConfig::SaveXmlFile()
{
    CvFileStorage* fs = cvOpenFileStorage(XML_FILE, 0, CV_STORAGE_WRITE);
    cvWriteInt(fs, "IpCam", _config.ipCam);
    cvWriteString(fs, "VsCamGV", _config.strCamUrl_GV.data());
    cvWriteString(fs, "IrCamGV", _config.strCamUrl_GV_ir.data());
    cvWriteString(fs, "VsCamIP", _config.strCamUrl_IP.data());
    cvWriteString(fs, "IrCamIP", _config.strCamUrl_IP_ir.data());
    cvWriteInt(fs, "TrkWidth", _config.trkWidth);
    cvWriteInt(fs, "TrkHeight", _config.trkHeight);
    cvWriteInt(fs, "FrmPosX", _config.frmPosX);
    cvWriteInt(fs, "FrmPosY", _config.frmPosY);
    cvWriteInt(fs, "FrmWidth", _config.frmWidth);
    cvWriteInt(fs, "FrmHeight", _config.frmHeight);
    cvWriteReal(fs, "Fps", _config.fps);
    cvReleaseFileStorage(&fs);
}
