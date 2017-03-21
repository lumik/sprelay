QT += serialport

SOURCES += \
    $$PWD/testk8090.cpp \
    $$PWD/../../src/core/k8090.cpp

HEADERS  += $$PWD/../../src/core/k8090.h

DESTDIR = $$PWD/../../bin

INCLUDEPATH += $$PWD/../../src/core
