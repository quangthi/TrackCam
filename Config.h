#ifndef CCONFIG_H
#define CCONFIG_H

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <string>
#include <QRect>
#include "Utility.h"
#include <QtGui>

#define CFG_FILE            "C:\\Program Files\\NHCamera\\config.isc"
#define VIDEO_PATH          "D://VideoRecord/"
#define CONF_PATH           "C://Program Files//NHCamera/"


struct Config_t
{
    std::string strCamUrl, strCamUrl_ir;
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
