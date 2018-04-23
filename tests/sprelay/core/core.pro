TARGET = core_test

QT += serialport

include(../../tests.pri)

SOURCES += \
    $$PWD/../../../src/sprelay/core/k8090.cpp \
    $$PWD/../../../src/sprelay/core/mock_serial_port.cpp \
    $$PWD/../../../src/sprelay/core/serial_port_utils.cpp \
    $$PWD/../../../src/sprelay/core/unified_serial_port.cpp \
    command_queue_test.cpp \
    core_test.cpp \
    k8090_test.cpp \
    serial_port_utils_test.cpp \
    unified_serial_port_test.cpp

HEADERS  += \
    $$PWD/../../../src/sprelay/core/command_queue.h \
    $$PWD/../../../src/sprelay/core/command_queue.tpp \
    $$PWD/../../../src/sprelay/core/k8090.h \
    $$PWD/../../../src/sprelay/core/mock_serial_port.h \
    $$PWD/../../../src/sprelay/core/unified_serial_port.h \
    $$PWD/../../../src/sprelay/core/serial_port_utils.h \
    command_queue_test.h \
    k8090_test.h \
    serial_port_utils_test.h \
    unified_serial_port_test.h \
    core_test_utils.h

DESTDIR = $$PWD/../../../bin

INCLUDEPATH += $$PWD/../../../src/sprelay/core
