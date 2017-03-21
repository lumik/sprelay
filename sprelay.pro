TEMPLATE = subdirs

CONFIG += ordered

test {
    # build only tests
    SUBDIRS += tests
} else {
    SUBDIRS = \
        src \
        tests

    OTHER_FILES += Doxyfile

    # Add custom "doc" target
    _CMD_SEP = ;
    win32:_CMD_SEP = &

    dox.target = doc
    dox.commands = \
        cd $$shell_path($$PWD) $$_CMD_SEP \
        doxygen Doxyfile
    dox.depends =

    QMAKE_EXTRA_TARGETS += dox
}
