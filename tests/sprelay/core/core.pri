QT += serialport

SOURCES += \
    $$PWD/testk8090.cpp \
    $$PWD/../../../src/sprelay/core/k8090.cpp

HEADERS  += $$PWD/../../../src/sprelay/core/k8090.h

DESTDIR = $$PWD/../../../bin

INCLUDEPATH += $$PWD/../../../src/sprelay/core
