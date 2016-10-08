#ifndef CCONFIG_H
#define CCONFIG_H

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <string>
#include <QRect>
#include <QtGui>
#include "Utility.h"

#define CFG_FILE            "C:\\Program Files\\NHCamera\\config.isc"
#define VIDEO_PATH          "D://VideoRecord/"
#define CONF_PATH           "C://Program Files//NHCamera/"
#define XML_FILE            "C://Program Files//NHCamera//config.xml"


struct Config_t
{
    std::string strCamUrl_GV, strCamUrl_GV_ir, strCamUrl_IP, strCamUrl_IP_ir;

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
    void SaveXmlFile();
    void LoadXmlFile();
};

#endif // CCONFIG_H
