QT       += core gui widgets serialport

TARGET = sprelay
TEMPLATE = app

include(../../sprelay.pri)
include(gui/gui.pri)
include(core/core.pri)

SOURCES += main.cpp

DESTDIR = $$PWD/../../bin

INCLUDEPATH += \
    $$PWD \
    gui \
    core
