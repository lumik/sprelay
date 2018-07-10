QT += testlib core
QT -= gui

TEMPLATE = app

DEFINES += SPRELAY_STANDALONE

COFIG += console
COFIG -= app_bundle

include($$sprelay_source_dir/sprelay.pri)
