#include "Config.h"

CConfig::CConfig()
{
    LoadConfigFile();
}

bool CConfig::LoadConfigFile()
{
    QFile file(CFG_FILE);
    if(!file.open(QIODevice::ReadOnly))
    {
        setDefault();
        return false;
    }
    QTextStream in(&file);
    QString line = in.readLine();           // camera url
    _config.strCamUrl = line.toStdString();
    line = in.readLine();                   // ir camera url
    _config.strCamUrl_ir = line.toStdString();
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

    if(_config.strCamUrl.size()<1)
    {
        _config.strCamUrl = CAM_URL;
    }

    if(_config.strCamUrl_ir.size()<1)
    {
        _config.strCamUrl = CAM_URL_IR;
    }

    file.close();
    return true;
}

void CConfig::setDefault()
{
    _config.frmWidth        = FRM_WIDTH;
    _config.frmHeight       = FRM_HEIGHT;
    _config.frmPosX         = FRM_POS_X;
    _config.frmPosY         = FRM_POS_Y;
    _config.trkWidth        = TRK_WIDTH;
    _config.trkHeight       = TRK_HEIGHT;
    _config.strCamUrl       = CAM_URL;
    _config.strCamUrl_ir    = CAM_URL_IR;
    _config.fps             = FPS;
    _config.ipCam           = 1;

    SaveToFile();
}

void CConfig::SaveToFile()
{
    QFile configFile(CFG_FILE);
    if(!configFile.open(QIODevice::WriteOnly))return;

    QTextStream outStream(&configFile);

    outStream<<_config.strCamUrl.data()<<"\n";
    outStream<<_config.strCamUrl_ir.data()<<"\n";
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
