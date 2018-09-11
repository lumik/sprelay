TARGET = core_test

QT += serialport

include(../../tests.pri)

SOURCES += \
    $$sprelay_source_dir/src/sprelay/core/concurent_command_queue.cpp \
    $$sprelay_source_dir/src/sprelay/core/k8090.cpp \
    $$sprelay_source_dir/src/sprelay/core/k8090_utils.cpp \
    $$sprelay_source_dir/src/sprelay/core/mock_serial_port.cpp \
    $$sprelay_source_dir/src/sprelay/core/serial_port_utils.cpp \
    $$sprelay_source_dir/src/sprelay/core/unified_serial_port.cpp \
    command_queue_test.cpp \
    core_test.cpp \
    k8090_test.cpp \
    serial_port_utils_test.cpp \
    unified_serial_port_test.cpp \
    mock_serial_port_test.cpp \
    k8090_utils_test.cpp

HEADERS += \
    $$sprelay_source_dir/tests/external/qtest_suite/include/lumik/qtest_suite/qtest_suite.h \
    $$sprelay_source_dir/src/sprelay/core/command_queue.h \
    $$sprelay_source_dir/src/sprelay/core/command_queue.tpp \
    $$sprelay_source_dir/src/sprelay/core/concurent_command_queue.h \
    $$sprelay_source_dir/src/sprelay/core/k8090.h \
    $$sprelay_source_dir/src/sprelay/core/k8090_commands.h \
    $$sprelay_source_dir/src/sprelay/core/k8090_defines.h \
    $$sprelay_source_dir/src/sprelay/core/k8090_utils.h \
    $$sprelay_source_dir/src/sprelay/core/mock_serial_port.h \
    $$sprelay_source_dir/src/sprelay/core/serial_port_defines.h \
    $$sprelay_source_dir/src/sprelay/core/serial_port_utils.h \
    $$sprelay_source_dir/src/sprelay/core/unified_serial_port.h \
    command_queue_test.h \
    k8090_test.h \
    serial_port_utils_test.h \
    unified_serial_port_test.h \
    mock_serial_port_test.h \
    core_test_utils.h \
    k8090_utils_test.h

DESTDIR = $$sprelay_source_dir/bin
target.path = $$sprelay_install_prefix/bin

!equals(sprelay_install_prefix, $$sprelay_source_dir) {
    INSTALLS += target
}
