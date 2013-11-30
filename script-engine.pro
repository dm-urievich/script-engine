#-------------------------------------------------
#
# Project created by QtCreator 2013-11-10T15:03:17
#
#-------------------------------------------------

QT       += core

QT       -= gui

QT += xml

TARGET = script-engine
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    eventmachinethread.cpp \
    connector.cpp

HEADERS += \
    eventmachinethread.h \
    connector.h

OTHER_FILES += \
    ../connections-conf.xml
