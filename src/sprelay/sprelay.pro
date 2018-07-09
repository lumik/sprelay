QT       += core gui widgets serialport

CONFIG += no_include_pwd

TARGET = sprelay

include(../../sprelay.pri)

sprelay_build_library {
    TEMPLATE = lib
    DEFINES += SPRELAY_BUILD_LIBRARY
    DESTDIR = $$PWD/../../lib
    target.path = $$qmake_install_prefix/lib

    # copy library headers
    sprelay_library_header_files_relative += \
        sprelay/core/k8090.h \
        sprelay/core/serial_port_defines.h \
        sprelay/gui/central_widget.h \
        sprelay/global.h
    sprelay_base_library_path += $$sprelay_source_dir/src
    QMAKE_POST_LINK += $$copyToDir($$sprelay_base_library_path, $$sprelay_library_header_files_relative, \
        $$sprelay_source_dir/include)

    # install library headers
    !equals(qmake_install_prefix, $$sprelay_source_dir) {
        sprelay_library_include.path = $$qmake_install_prefix/include
        sprelay_library_include.files = $$sprelay_source_dir/include/*
        INSTALLS += sprelay_library_include
    }

} else {
    TEMPLATE = app
    DEFINES += SPRELAY_STANDALONE
    DESTDIR = sprelay_source_dir/bin
    target.path = $$qmake_install_prefix/bin
}

!equals(qmake_install_prefix, $$sprelay_source_dir) {
    INSTALLS += target
}

include(gui/gui.pri)
include(core/core.pri)

SOURCES += main.cpp

INCLUDEPATH += \
    $$PWD/..

HEADERS += \
    sprelay_global.h
