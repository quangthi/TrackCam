#-------------------------------------------------
#
# Project created by Nguyen Quang Thi 2016-06-19T11:16:37
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TrackCam
TEMPLATE = app

RC_FILE = TrackCam.rc


SOURCES += main.cpp\
        mainwindow.cpp \
    Utility.cpp \
    Tracker.cpp \
    videodisplay.cpp \
    qcustombutton.cpp \
    qcustomcombobox.cpp \
    qcustomframe.cpp \
    qcustomgroupbox.cpp \
    qcustomtabwidget.cpp \
    Config.cpp \
    qcustomedit.cpp \
    qcustomradio.cpp \
    qcustomcheckbox.cpp \
    videowork.cpp

HEADERS  += mainwindow.h \
    Utility.h \
    Tracker.h \
    videodisplay.h \
    qcustombutton.h \
    qcustomcombobox.h \
    qcustomframe.h \
    qcustomgroupbox.h \
    qcustomtabwidget.h \
    Config.h \
    qcustomedit.h \
    qcustomradio.h \
    qcustomcheckbox.h \
    videowork.h

FORMS    += mainwindow.ui \
    videodisplay.ui

#win32:LIBS += -L$$PWD/../armadilloWin32/lib_winx86/ -lblas_win32_MT
#win32:LIBS += -L$$PWD/../armadilloWin32/lib_winx86/ -llapack_win32_MT

INCLUDEPATH += $$PWD/../opencv/build/include
#debug:
win32:CONFIG(debug, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_core2413d
win32:CONFIG(debug, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_highgui2413d
win32:CONFIG(debug, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_imgproc2413d
win32:CONFIG(debug, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_calib3d2413d
win32:CONFIG(debug, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_contrib2413d
win32:CONFIG(debug, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_core2413d
win32:CONFIG(debug, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_features2d2413d
win32:CONFIG(debug, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_flann2413d
win32:CONFIG(debug, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_gpu2413d
win32:CONFIG(debug, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_legacy2413d
win32:CONFIG(debug, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_ml2413d
win32:CONFIG(debug, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_nonfree2413d
win32:CONFIG(debug, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_objdetect2413d
win32:CONFIG(debug, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_ocl2413d
win32:CONFIG(debug, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_photo2413d
win32:CONFIG(debug, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_stitching2413d
win32:CONFIG(debug, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_superres2413d
win32:CONFIG(debug, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_video2413d
win32:CONFIG(debug, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_videostab2413d
#release:
win32:CONFIG(release, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_core2413
win32:CONFIG(release, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_highgui2413
win32:CONFIG(release, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_imgproc2413
win32:CONFIG(release, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_calib3d2413
win32:CONFIG(release, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_contrib2413
win32:CONFIG(release, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_core2413
win32:CONFIG(release, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_features2d2413
win32:CONFIG(release, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_flann2413
win32:CONFIG(release, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_gpu2413
win32:CONFIG(release, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_legacy2413
win32:CONFIG(release, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_ml2413
win32:CONFIG(release, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_nonfree2413
win32:CONFIG(release, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_objdetect2413
win32:CONFIG(release, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_ocl2413
win32:CONFIG(release, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_photo2413
win32:CONFIG(release, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_stitching2413
win32:CONFIG(release, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_superres2413
win32:CONFIG(release, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_video2413
win32:CONFIG(release, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc11/lib/ -lopencv_videostab2413
