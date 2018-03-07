TARGET = core_test

QT += serialport

include(../../tests.pri)

SOURCES += \
    $$PWD/../../../src/sprelay/core/k8090.cpp \
    $$PWD/command_queue_test.cpp \
    $$PWD/k8090_test.cpp \
    core_test.cpp

HEADERS  += $$PWD/../../../src/sprelay/core/k8090.h \
            $$PWD/../../../src/sprelay/core/command_queue.h \
            $$PWD/../../../src/sprelay/core/command_queue.tpp \
    command_queue_test.h \
    k8090_test.h

DESTDIR = $$PWD/../../../bin

INCLUDEPATH += $$PWD/../../../src/sprelay/core
