QT += testlib
QT -= gui

TARGET = sprelayunittests
TEMPLATE = app

COFIG += console
COFIG -= app_bundle

include(../sprelay.pri)
include(core/core.pri)
