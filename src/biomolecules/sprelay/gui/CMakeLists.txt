set(sprelay_gui_project_name ${sprelay_project_name})

# set some globals
set(sprelay_gui_install_include_dir "${sprelay_install_include_dir}/gui")

# collect files
set(${sprelay_gui_project_name}_lib_hdr)
set(${sprelay_gui_project_name}_lib_tpp)
set(${sprelay_gui_project_name}_lib_qt_hdr
    central_widget.h)
set(${sprelay_gui_project_name}_lib_src
    central_widget.cpp)
set(${sprelay_gui_project_name}_hdr)
set(${sprelay_gui_project_name}_qt_hdr
    indicator_button.h)
set(${sprelay_gui_project_name}_tpp)
set(${sprelay_gui_project_name}_src
    indicator_button.cpp)
set(${sprelay_gui_project_name}_ui)

# compile files connected only with standalone application only on demand
if (BUILD_STANDALONE)
    list(APPEND ${sprelay_gui_project_name}_qt_hdr
        main_window.h)
    list(APPEND ${sprelay_gui_project_name}_src
        main_window.cpp)
endif()

# create build and install file paths
foreach(hdr ${${sprelay_gui_project_name}_lib_hdr})
    list(APPEND ${sprelay_gui_project_name}_lib_hdr_build "${CMAKE_CURRENT_LIST_DIR}/${hdr}")
    list(APPEND ${sprelay_gui_project_name}_lib_hdr_install
        "${sprelay_gui_install_include_dir}/${hdr}" )
endforeach()
foreach(hdr ${${sprelay_gui_project_name}_lib_tpp})
    list(APPEND ${sprelay_gui_project_name}_lib_tpp_build "${CMAKE_CURRENT_LIST_DIR}/${hdr}")
    list(APPEND ${sprelay_gui_project_name}_lib_tpp_install
        "${sprelay_gui_install_include_dir}/${hdr}")
endforeach()
foreach(hdr ${${sprelay_gui_project_name}_lib_qt_hdr})
    list(APPEND ${sprelay_gui_project_name}_lib_qt_hdr_build "${CMAKE_CURRENT_LIST_DIR}/${hdr}")
    list(APPEND ${sprelay_gui_project_name}_lib_qt_hdr_install
        "${sprelay_gui_install_include_dir}/${hdr}" )
endforeach()
foreach(src ${${sprelay_gui_project_name}_lib_src})
    list(APPEND ${sprelay_gui_project_name}_lib_src_build "${CMAKE_CURRENT_LIST_DIR}/${src}")
endforeach()
foreach(hdr ${${sprelay_gui_project_name}_hdr})
    list(APPEND ${sprelay_gui_project_name}_hdr_build "${CMAKE_CURRENT_LIST_DIR}/${hdr}")
endforeach()
foreach(hdr ${${sprelay_gui_project_name}_tpp})
    list(APPEND ${sprelay_gui_project_name}_tpp_build "${CMAKE_CURRENT_LIST_DIR}/${hdr}")
endforeach()
foreach(hdr ${${sprelay_gui_project_name}_qt_hdr})
    list(APPEND ${sprelay_gui_project_name}_qt_hdr_build "${CMAKE_CURRENT_LIST_DIR}/${hdr}")
endforeach()
foreach(src ${${sprelay_gui_project_name}_src})
    list(APPEND ${sprelay_gui_project_name}_src_build "${CMAKE_CURRENT_LIST_DIR}/${src}")
endforeach()
foreach(src ${${sprelay_gui_project_name}_ui})
    list(APPEND ${sprelay_gui_project_name}_ui_build "${CMAKE_CURRENT_LIST_DIR}/${src}")
endforeach()

# call qt moc
qt5_wrap_cpp(${sprelay_gui_project_name}_hdr_moc_build ${${sprelay_gui_project_name}_qt_hdr_build})
qt5_wrap_cpp(${sprelay_gui_project_name}_lib_hdr_moc_build ${${sprelay_gui_project_name}_lib_qt_hdr_build})
qt5_wrap_ui(${sprelay_gui_project_name}_ui_moc_build ${${sprelay_gui_project_name}_ui_build})

# continue to create executable or dll
if (BUILD_STANDALONE)
    # add sources
    target_sources(${sprelay_gui_project_name} PRIVATE
        $<BUILD_INTERFACE:${${sprelay_gui_project_name}_lib_src_build}>
        $<BUILD_INTERFACE:${${sprelay_gui_project_name}_lib_hdr_moc_build}>
        $<BUILD_INTERFACE:${${sprelay_gui_project_name}_src_build}>
        $<BUILD_INTERFACE:${${sprelay_gui_project_name}_hdr_moc_build}>
        $<BUILD_INTERFACE:${${sprelay_gui_project_name}_ui_moc_build}>)

    # attach header files to the library (mainly to display them in IDEs)
    target_sources(${sprelay_gui_project_name} PRIVATE
        $<BUILD_INTERFACE:${${sprelay_gui_project_name}_lib_hdr_build}>
        $<BUILD_INTERFACE:${${sprelay_gui_project_name}_lib_tpp_build}>
        $<BUILD_INTERFACE:${${sprelay_gui_project_name}_lib_qt_hdr_build}>
        $<BUILD_INTERFACE:${${sprelay_gui_project_name}_hdr_build}>
        $<BUILD_INTERFACE:${${sprelay_gui_project_name}_tpp_build}>
        $<BUILD_INTERFACE:${${sprelay_gui_project_name}_qt_hdr_build}>)
else()
    # add sources
    target_sources(${sprelay_gui_project_name}
        PRIVATE
            $<BUILD_INTERFACE:${${sprelay_gui_project_name}_lib_src_build}>
            $<BUILD_INTERFACE:${${sprelay_gui_project_name}_lib_hdr_moc_build}>
            $<BUILD_INTERFACE:${${sprelay_gui_project_name}_src_build}>
            $<BUILD_INTERFACE:${${sprelay_gui_project_name}_hdr_moc_build}>
            $<BUILD_INTERFACE:${${sprelay_gui_project_name}_ui_moc_build}>)

    # attach header files to the library (mainly to display them in IDEs)
    target_sources(${sprelay_gui_project_name}
        PUBLIC
            $<INSTALL_INTERFACE:${${sprelay_gui_project_name}_lib_hdr_install}>
            $<BUILD_INTERFACE:${${sprelay_gui_project_name}_lib_hdr_build}>
            $<INSTALL_INTERFACE:${${sprelay_gui_project_name}_lib_tpp_install}>
            $<BUILD_INTERFACE:${${sprelay_gui_project_name}_lib_tpp_build}>
            $<INSTALL_INTERFACE:${${sprelay_gui_project_name}_lib_qt_hdr_install}>
            $<BUILD_INTERFACE:${${sprelay_gui_project_name}_lib_qt_hdr_build}>
        PRIVATE
            $<BUILD_INTERFACE:${${sprelay_gui_project_name}_hdr_build}>
            $<BUILD_INTERFACE:${${sprelay_gui_project_name}_tpp_build}>
            $<BUILD_INTERFACE:${${sprelay_gui_project_name}_qt_hdr_build}>)
    install(FILES
            ${${sprelay_gui_project_name}_lib_hdr_build}
            ${${sprelay_gui_project_name}_lib_qt_hdr_build}
        DESTINATION ${sprelay_gui_install_include_dir})
endif()

# finish the gui addition
target_include_directories(${sprelay_gui_project_name} PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${sprelay_source_dir}>)
target_link_libraries(${sprelay_gui_project_name}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Threads::Threads
    lumik::enum_flags::enum_flags
    biomolecules::sprelay::sprelay_globals
    biomolecules::sprelay::sprelay_core)

if (DOXYGEN_FOUND)
    set(${sprelay_gui_project_name}_doc_src
        ${CMAKE_CURRENT_LIST_DIR}/gui.dox)
    target_sources(${sprelay_gui_project_name} PRIVATE ${${sprelay_gui_project_name}_doc_src})
    set_source_files_properties(${${sprelay_gui_project_name}_doc_src} PROPERTIES HEADER_FILE_ONLY TRUE)
endif()
