QT += testlib core
QT -= gui

TEMPLATE = app

DEFINES += SPRELAY_STANDALONE

CONFIG += \
    console \
    testcase
CONFIG -= app_bundle

include($$sprelay_source_dir/sprelay.pri)

INCLUDEPATH += \
    $$sprelay_source_dir/tests \
    $$sprelay_source_dir/src
