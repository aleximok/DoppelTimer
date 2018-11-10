#-------------------------------------------------
#
# Project created by QtCreator 2012-11-20T19:58:32
#
#-------------------------------------------------

QT       += core gui sql widgets multimedia

TARGET = DoppelTimer
TEMPLATE = app

CONFIG += static
CONFIG += c++11

DEFINES += APPNAME=\\\"DoppelTimer\\\"
DEFINES += APPVERSION=\\\"1.1\\\"

SOURCES += main.cpp\
    CTimerWindow.cpp \
    CDigitalTimer.cpp \
    CSounder.cpp \
    CActivityDialog.cpp \
    CActivityDelegate.cpp \
    CWindowSticker.cpp \
    CDbOperations.cpp

HEADERS  += CTimerWindow.h \
    CDigitalTimer.h \
    CSounder.h \
    CActivityDialog.h \
    CActivityDelegate.h \
    CWindowSticker.h \
    CDbOperations.h

FORMS    += CTimerWindow.ui \
    CActivityDialog.ui

RESOURCES += \
    DoppelTimer.qrc

RC_FILE = DoppelTimer.rc
