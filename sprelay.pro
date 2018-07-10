TEMPLATE = subdirs

test {
    # build only tests
    SUBDIRS += \
        tests
} else {
    SUBDIRS += \
        src \
        tests

    tests.depends = src

    # Add custom "doc" target
    _CMD_SEP = ;
    win32:_CMD_SEP = &

    dox.target = doc
    dox.commands = \
        cd $$shell_path($$PWD) $$_CMD_SEP \
        doxygen Doxyfile
    dox.depends =

    !equals(sprelay_install_prefix, $$sprelay_source_dir) {
        documentation.files = doc/*
        documentation.path = $$sprelay_install_prefix/doc
        INSTALLS += documentation
    }

    QMAKE_EXTRA_TARGETS += dox
}

OTHER_FILES += \
    Doxyfile

DISTFILES += \
    .qmake.conf
