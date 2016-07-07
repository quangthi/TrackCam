#ifndef CCONFIG_H
#define CCONFIG_H

#define CFG_FILE        "config.isc"
#define FRM_WIDTH       376
#define FRM_HEIGHT      290
#define FRM_POS_X       645
#define FRM_POS_Y       420
#define CAM_URL         "rtsp://127.0.0.1:8554/cam1_stream1"
#define TRK_WIDTH       90
#define TRK_HEIGHT      60
#define FPS             24
#define LOCAL_UDP_PORT  8000
#define REMOTE_UDP_PORT 8001


#include <QFile>
#include <QTextStream>
#include <string>

struct Config_t
{
    std::string strCamUrl;
    short frmWidth, frmHeight, frmPosX, frmPosY, trkWidth, trkHeight, fps, ipCam;
};

class CConfig
{
public:
    Config_t _config;
public:
    CConfig();
    bool LoadConfigFile();
    void setDefault();
    void SaveToFile();
};

#endif // CCONFIG_H
