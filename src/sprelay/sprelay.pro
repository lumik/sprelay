QT     += core gui widgets serialport

CONFIG += no_include_pwd

TARGET = sprelay

include($$sprelay_source_dir/sprelay.pri)

sprelay_build_standalone {
    TEMPLATE = app
    DEFINES += SPRELAY_STANDALONE
    DESTDIR = $$sprelay_source_dir/bin
    target.path = $$sprelay_install_prefix/bin
} else {
    TEMPLATE = lib
    DEFINES += SPRELAY_BUILD_LIBRARY
    DESTDIR = $$sprelay_source_dir/lib
    target.path = $$sprelay_install_prefix/lib

    # copy library headers
    sprelay_library_header_files_relative += \
        sprelay/core/k8090.h \
        sprelay/core/serial_port_defines.h \
        sprelay/gui/central_widget.h \
        sprelay/sprelay_global.h
    sprelay_base_library_path += $$sprelay_source_dir/src
    QMAKE_POST_LINK += $$copyToDir($$sprelay_base_library_path, $$sprelay_library_header_files_relative, \
        $$sprelay_source_dir/include)

    # install library headers
    !equals(sprelay_install_prefix, $$sprelay_source_dir) {
        sprelay_library_include.path = $$sprelay_install_prefix/include
        sprelay_library_include.files = $$sprelay_source_dir/include/*
        INSTALLS += sprelay_library_include
    }
}

!equals(sprelay_install_prefix, $$sprelay_source_dir) {
    INSTALLS += target
}

include(gui/gui.pri)
include(core/core.pri)

SOURCES += main.cpp

INCLUDEPATH += \
    $$PWD/..

HEADERS += \
    sprelay_global.h
