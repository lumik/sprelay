QT       += core gui widgets serialport

CONFIG += no_include_pwd

TARGET = sprelay
TEMPLATE = app

include(../../sprelay.pri)
include(gui/gui.pri)
include(core/core.pri)

SOURCES += main.cpp

DESTDIR = $$PWD/../../bin

INCLUDEPATH += \
    $$PWD/..
