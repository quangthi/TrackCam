#ifndef CCONFIG_H
#define CCONFIG_H


#include <QFile>
#include <QTextStream>
#include <string>
#include "Utility.h"

#define CFG_FILE        "config.isc"


struct Config_t
{
    std::string strCamUrl;
    short frmWidth, frmHeight, frmPosX, frmPosY, trkWidth, trkHeight, ipCam;
    double fps;
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
