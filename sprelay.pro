TEMPLATE = subdirs

CONFIG += ordered

test {
    # build only tests
    SUBDIRS += tests
} else {
    SUBDIRS += \
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

    !equals(qmake_install_prefix, $$sprelay_source_dir) {
        dox.files = doc/*
        dox.path = $$qmake_install_prefix/doc
        INSTALLS += dox
    }

    QMAKE_EXTRA_TARGETS += dox
}

OTHER_FILES += \
    .qmake.conf
