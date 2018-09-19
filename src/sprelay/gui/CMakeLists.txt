set(sprelay_core_project_name ${sprelay_project_name}_core)

if(SPRELAY_BUILD_STANDALONE OR NOT SPRELAY_SKIP_GUI)
    # collect files
    set(${sprelay_core_project_name}_lib_hdr)
    set(${sprelay_core_project_name}_lib_qt_hdr
        ${CMAKE_CURRENT_LIST_DIR}/central_widget.h)
    set(${sprelay_core_project_name}_lib_tpp)
    set(${sprelay_core_project_name}_hdr)
    set(${sprelay_core_project_name}_qt_hdr
        ${CMAKE_CURRENT_LIST_DIR}/indicator_button.h)
    set(${sprelay_core_project_name}_tpp)
    set(${sprelay_core_project_name}_src
        ${CMAKE_CURRENT_LIST_DIR}/central_widget.cpp
        ${CMAKE_CURRENT_LIST_DIR}/indicator_button.cpp)
    set(${sprelay_core_project_name}_ui)

    # compile files connected only with standalone application only on demand
    if(SPRELAY_BUILD_STANDALONE)
        list(APPEND ${sprelay_core_project_name}_qt_hdr
            ${CMAKE_CURRENT_LIST_DIR}/main_window.h)
        list(APPEND ${sprelay_core_project_name}_src
            ${CMAKE_CURRENT_LIST_DIR}/main_window.cpp)
    endif(SPRELAY_BUILD_STANDALONE)

    # call qt moc
    qt5_wrap_cpp(${sprelay_core_project_name}_hdr_moc ${${sprelay_core_project_name}_qt_hdr})
    qt5_wrap_cpp(${sprelay_core_project_name}_lib_hdr_moc ${${sprelay_core_project_name}_lib_qt_hdr})
    qt5_wrap_ui(${sprelay_core_project_name}_ui_moc ${${sprelay_core_project_name}_ui})

    # create executable or dll
    if(SPRELAY_BUILD_STANDALONE)
        target_sources(${sprelay_project_name} PRIVATE
            $<BUILD_INTERFACE:${${sprelay_core_project_name}_lib_hdr}>
            $<BUILD_INTERFACE:${${sprelay_core_project_name}_lib_tpp}>
            $<BUILD_INTERFACE:${${sprelay_core_project_name}_lib_qt_hdr}>
            $<BUILD_INTERFACE:${${sprelay_core_project_name}_lib_hdr_moc}>
            $<BUILD_INTERFACE:${${sprelay_core_project_name}_hdr}>
            $<BUILD_INTERFACE:${${sprelay_core_project_name}_tpp}>
            $<BUILD_INTERFACE:${${sprelay_core_project_name}_qt_hdr}>
            $<BUILD_INTERFACE:${${sprelay_core_project_name}_src}>
            $<BUILD_INTERFACE:${${sprelay_core_project_name}_hdr_moc}>
            $<BUILD_INTERFACE:${${sprelay_core_project_name}_ui_moc}>)
    else()
        target_sources(${sprelay_project_name} PUBLIC
            ${${sprelay_core_project_name}_lib_hdr}
            ${${sprelay_core_project_name}_lib_tpp}
            ${${sprelay_core_project_name}_lib_qt_hdr})
        target_sources(${sprelay_project_name} PRIVATE
            $<BUILD_INTERFACE:${${sprelay_core_project_name}_lib_hdr_moc}>
            $<BUILD_INTERFACE:${${sprelay_core_project_name}_hdr}>
            $<BUILD_INTERFACE:${${sprelay_core_project_name}_tpp}>
            $<BUILD_INTERFACE:${${sprelay_core_project_name}_qt_hdr}>
            $<BUILD_INTERFACE:${${sprelay_core_project_name}_src}>
            $<BUILD_INTERFACE:${${sprelay_core_project_name}_hdr_moc}>
            $<BUILD_INTERFACE:${${sprelay_core_project_name}_ui_moc}>)
        install(FILES ${${sprelay_core_project_name}_lib_hdr} ${${sprelay_core_project_name}_lib_qt_hdr}
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sprelay/gui)
    endif(SPRELAY_BUILD_STANDALONE)
    target_include_directories(${sprelay_project_name} SYSTEM PUBLIC $<INSTALL_INTERFACE:include>)
    target_include_directories(${sprelay_project_name} PUBLIC $<BUILD_INTERFACE:${sprelay_source_dir}>)
    target_link_libraries(${sprelay_project_name}
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Threads::Threads
        enum_flags
        sprelay_global
        sprelay_core)

    if(DOXYGEN_FOUND)
        set(${sprelay_project_name}_doc_src
            ${CMAKE_CURRENT_LIST_DIR}/gui.dox)
        target_sources(${sprelay_project_name} PRIVATE ${${sprelay_project_name}_doc_src})
        set_source_files_properties(${${sprelay_project_name}_doc_src} PROPERTIES HEADER_FILE_ONLY TRUE)
    endif(DOXYGEN_FOUND)

endif(SPRELAY_BUILD_STANDALONE OR NOT SPRELAY_SKIP_GUI)
