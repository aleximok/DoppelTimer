#-------------------------------------------------
#
# Project created by QtCreator 2012-11-20T19:58:32
#
#-------------------------------------------------

QT       += core gui sql widgets multimedia

TARGET = DoppelTimer
TEMPLATE = app

CONFIG += static
CONFIG += c++14

VERSION = "1.3"
AUTHOR  = "Oleksii Mokrintsev"

QMAKE_TARGET_PRODUCT = DoppelTimer
QMAKE_TARGET_DESCRIPTION = Timer tracking app
QMAKE_TARGET_COPYRIGHT = Copyright (C) $$AUTHOR

RC_ICONS = DoppelTimer.ico

DEFINES += AUTHOR=\'\"$${AUTHOR}\"\'
DEFINES += APPNAME=\\\"DoppelTimer\\\"
DEFINES += APPVERSION=\\\"$$VERSION\\\"

SOURCES += main.cpp\
    CTimerWindow.cpp \
    CDigitalTimer.cpp \
    CSounder.cpp \
    CActivityDialog.cpp \
    CActivityDelegate.cpp \
    CWindowSticker.cpp \
    CDbOperations.cpp \
    CGLSkyCube.cpp \
    CValueTransition.cpp \
    CSkyScreensaver.cpp

HEADERS  += CTimerWindow.h \
    CDigitalTimer.h \
    CSounder.h \
    CActivityDialog.h \
    CActivityDelegate.h \
    CWindowSticker.h \
    CDbOperations.h \
    CGLSkyCube.h \
    CValueTransition.h \
    CSkyScreensaver.h

FORMS    += CTimerWindow.ui \
    CActivityDialog.ui

RESOURCES += \
    DoppelTimer.qrc

#RC_FILE = DoppelTimer.rc
