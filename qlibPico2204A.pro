#-------------------------------------------------
#
# Project created by QtCreator 2016-02-06T09:28:21
#
#-------------------------------------------------

QT       += core gui

TARGET = qlibPico2204A
TEMPLATE = lib
CONFIG += plugin

SOURCES += qlibpico2204a.cpp

HEADERS += qlibpico2204a.h \
    qlibpico2204a_interface.h \
    qlibpicowave_data_format.h
OTHER_FILES += qlibPico2204A.json

#Picoscope ps2000 lib
unix:  LIBS        += -L/opt/picoscope/lib -lps2000
unix:  INCLUDEPATH += "/opt/picoscope/include/libps2000-2.1"
win32: LIBS        += -L$$_PRO_FILE_PWD_/ps2000/ -lps2000
win32: INCLUDEPATH += $$_PRO_FILE_PWD_/ps2000/inc/
