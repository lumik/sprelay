QT       += core gui widgets serialport

CONFIG += no_include_pwd

TARGET = sprelay

sprelay_build_library {
    TEMPLATE = lib
    DEFINES += SPRELAY_BUILD_LIBRARY
    DESTDIR = $$PWD/../../lib
} else {
    TEMPLATE = app
    DEFINES += SPRELAY_STANDALONE
    DESTDIR = $$PWD/../../bin
}

include(../../sprelay.pri)
include(gui/gui.pri)
include(core/core.pri)

SOURCES += main.cpp

INCLUDEPATH += \
    $$PWD/..

HEADERS += \
    sprelay_global.h
