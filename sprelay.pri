CONFIG += c++11

INCLUDEPATH += $$PWD/src/external/enum_flags/src

defineReplace(copyToDir) {
    base = $$1
    files = $$2
    dest_dir = $$3
    link =
    destination_dirs =

    # create directory structure at first
    for(file, files) {
        out_filename = $$clean_path($$dest_dir/$$file)
        destination_parent = $$shell_path($$dirname(out_filename))
        !contains(destination_dirs, $$destination_parent) {
            destination_dirs += $$destination_parent
            link += $$sprintf($$QMAKE_MKDIR_CMD, "$$destination_parent") $$escape_expand(\\n\\t)
        }
    }
    for(file, files) {
        link += $$QMAKE_COPY $$shell_path($$clean_path($$base/$$file)) $$shell_path($$clean_path($$dest_dir/$$file)) \
            $$escape_expand(\\n\\t)
    }
    return($$link)
}
