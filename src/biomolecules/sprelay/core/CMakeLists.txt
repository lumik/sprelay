project(${sprelay_project_name}_core)

# set some globals
set(sprelay_core_install_include_dir "${sprelay_install_include_dir}/core")

# collect files
set(${PROJECT_NAME}_lib_hdr
    k8090_defines.h
    serial_port_defines.h)
set(${PROJECT_NAME}_lib_tpp)
set(${PROJECT_NAME}_lib_qt_hdr
    k8090.h)
set(${PROJECT_NAME}_lib_src
    k8090.cpp)
set(${PROJECT_NAME}_hdr
    command_queue.h
    concurent_command_queue.h
    k8090_commands.h
    k8090_utils.h
    serial_port_utils.h)
set(${PROJECT_NAME}_tpp
    command_queue.tpp)
set(${PROJECT_NAME}_qt_hdr
    mock_serial_port.h
    unified_serial_port.h)
set(${PROJECT_NAME}_src
    concurent_command_queue.cpp
    k8090_utils.cpp
    mock_serial_port.cpp
    serial_port_utils.cpp
    unified_serial_port.cpp)
set(${PROJECT_NAME}_ui)

# create build and install file paths
foreach(hdr ${${PROJECT_NAME}_lib_hdr})
    list(APPEND ${PROJECT_NAME}_lib_hdr_build "${PROJECT_SOURCE_DIR}/${hdr}")
    list(APPEND ${PROJECT_NAME}_lib_hdr_install "${sprelay_core_install_include_dir}/${hdr}")
endforeach()
foreach(hdr ${${PROJECT_NAME}_lib_tpp})
    list(APPEND ${PROJECT_NAME}_lib_tpp_build "${PROJECT_SOURCE_DIR}/${hdr}")
    list(APPEND ${PROJECT_NAME}_lib_tpp_install "${sprelay_core_install_include_dir}/${hdr}")
endforeach()
foreach(hdr ${${PROJECT_NAME}_lib_qt_hdr})
    list(APPEND ${PROJECT_NAME}_lib_qt_hdr_build "${PROJECT_SOURCE_DIR}/${hdr}")
    list(APPEND ${PROJECT_NAME}_lib_qt_hdr_install "${sprelay_core_install_include_dir}/${hdr}")
endforeach()
foreach(src ${${PROJECT_NAME}_lib_src})
    list(APPEND ${PROJECT_NAME}_lib_src_build "${PROJECT_SOURCE_DIR}/${src}")
endforeach()
foreach(hdr ${${PROJECT_NAME}_hdr})
    list(APPEND ${PROJECT_NAME}_hdr_build "${PROJECT_SOURCE_DIR}/${hdr}")
endforeach()
foreach(hdr ${${PROJECT_NAME}_tpp})
    list(APPEND ${PROJECT_NAME}_tpp_build "${PROJECT_SOURCE_DIR}/${hdr}")
endforeach()
foreach(hdr ${${PROJECT_NAME}_qt_hdr})
    list(APPEND ${PROJECT_NAME}_qt_hdr_build "${PROJECT_SOURCE_DIR}/${hdr}")
endforeach()
foreach(src ${${PROJECT_NAME}_src})
    list(APPEND ${PROJECT_NAME}_src_build "${PROJECT_SOURCE_DIR}/${src}")
endforeach()
foreach(src ${${PROJECT_NAME}_ui})
    list(APPEND ${PROJECT_NAME}_ui_build "${PROJECT_SOURCE_DIR}/${src}")
endforeach()

# call qt moc
qt5_wrap_cpp(${PROJECT_NAME}_hdr_moc_build ${${PROJECT_NAME}_qt_hdr_build})
qt5_wrap_cpp(${PROJECT_NAME}_lib_hdr_moc_build ${${PROJECT_NAME}_lib_qt_hdr_build})
qt5_wrap_ui(${PROJECT_NAME}_ui_moc_build ${${PROJECT_NAME}_ui_build})


# object files - can be used in tests #
# ----------------------------------- #

if (use_object_targets)
    set(${PROJECT_NAME}_private_target ${PROJECT_NAME}_private)

    # create the library
    add_library(${${PROJECT_NAME}_private_target} OBJECT
        ${${PROJECT_NAME}_src_build}
        ${${PROJECT_NAME}_hdr_moc_build}
        ${${PROJECT_NAME}_ui_moc_build})
    target_link_libraries(${${PROJECT_NAME}_private_target}
        Qt5::Core
        Qt5::SerialPort
        Threads::Threads
        lumik::enum_flags::enum_flags
        biomolecules::sprelay::sprelay_globals)
    target_include_directories(${${PROJECT_NAME}_private_target} PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${sprelay_source_dir}>)

    # attach header files to the library (mainly to display them in IDEs)
    target_sources(${${PROJECT_NAME}_private_target} PUBLIC
        $<BUILD_INTERFACE:${${PROJECT_NAME}_lib_hdr_build}>
        $<BUILD_INTERFACE:${${PROJECT_NAME}_lib_tpp_build}>
        $<BUILD_INTERFACE:${${PROJECT_NAME}_lib_qt_hdr_build}>
        $<BUILD_INTERFACE:${${PROJECT_NAME}_hdr_build}>
        $<BUILD_INTERFACE:${${PROJECT_NAME}_tpp_build}>
        $<BUILD_INTERFACE:${${PROJECT_NAME}_qt_hdr_build}>)

    # create alias to enable treating the library inside this project as if it were imported in namespace
    add_library(biomolecules::${sprelay_project_name}::${${PROJECT_NAME}_private_target}
        ALIAS ${${PROJECT_NAME}_private_target})

    if (NOT BUILD_STANDALONE)
        # install the library and export it to target list
        install(TARGETS ${${PROJECT_NAME}_private_target} EXPORT ${sprelay_project_name}Targets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()
# TODO(lumik): interface targets can't be used as the replacement for object files for older cmake versions because
# problems with qt moc, see comments in tests. Devise better approach to define used headers here.
#else()
#    set(${PROJECT_NAME}_private_target ${PROJECT_NAME}_private)

#    # create the library
#    add_library(${${PROJECT_NAME}_private_target} INTERFACE)
##    set_property(TARGET ${${PROJECT_NAME}_private_target} PROPERTY qt_hdr ${${PROJECT_NAME}_qt_hdr})
##    set_property(TARGET ${${PROJECT_NAME}_private_target} PROPERTY qt_ui ${${PROJECT_NAME}_ui})
#    target_sources(${${PROJECT_NAME}_private_target} INTERFACE
#        ${${PROJECT_NAME}_src_build}
#        ${${PROJECT_NAME}_hdr_moc_build}
#        ${${PROJECT_NAME}_ui_moc_build})
#    target_link_libraries(${${PROJECT_NAME}_private_target} INTERFACE
#        Qt5::Core
#        Qt5::SerialPort
#        Threads::Threads
#        lumik::enum_flags::enum_flags
#        biomolecules::sprelay::sprelay_globals)
#    target_include_directories(${${PROJECT_NAME}_private_target} PUBLIC
#        $<INSTALL_INTERFACE:include>
#        $<BUILD_INTERFACE:${sprelay_source_dir}>)
#
#    # attach header files to the library (mainly to display them in IDEs)
#    target_sources(${${PROJECT_NAME}_private_target} INTERFACE
#        $<BUILD_INTERFACE:${${PROJECT_NAME}_lib_hdr_build}>
#        $<BUILD_INTERFACE:${${PROJECT_NAME}_lib_tpp_build}>
#        $<BUILD_INTERFACE:${${PROJECT_NAME}_lib_qt_hdr_build}>
#        $<BUILD_INTERFACE:${${PROJECT_NAME}_hdr_build}>
#        $<BUILD_INTERFACE:${${PROJECT_NAME}_tpp_build}>
#        $<BUILD_INTERFACE:${${PROJECT_NAME}_qt_hdr_build}>)
#    if (NOT BUILD_STANDALONE)
#        # install the library and export it to target list
#        install(TARGETS ${${PROJECT_NAME}_private_target} EXPORT ${sprelay_project_name}Targets
#            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
#    endif()
endif()


# core library #
# ------------ #

if (use_object_targets)
    # create the library
    add_library(${PROJECT_NAME} SHARED
        ${${PROJECT_NAME}_lib_src_build}
        ${${PROJECT_NAME}_lib_hdr_moc_build})
    target_link_libraries(${PROJECT_NAME}
        Qt5::Core
        Qt5::SerialPort
        Threads::Threads
        lumik::enum_flags::enum_flags
        biomolecules::sprelay::sprelay_globals
        biomolecules::sprelay::${${PROJECT_NAME}_private_target})

    # attach header files to the library (mainly to display them in IDEs)
    target_sources(${PROJECT_NAME} PUBLIC
        $<INSTALL_INTERFACE:${${PROJECT_NAME}_lib_hdr_install}>
        $<BUILD_INTERFACE:${${PROJECT_NAME}_lib_hdr_build}>
        $<INSTALL_INTERFACE:${${PROJECT_NAME}_lib_tpp_install}>
        $<BUILD_INTERFACE:${${PROJECT_NAME}_lib_tpp_build}>
        $<INSTALL_INTERFACE:${${PROJECT_NAME}_lib_qt_hdr_install}>
        $<BUILD_INTERFACE:${${PROJECT_NAME}_lib_qt_hdr_build}>)
else()
#    # create the library
#    # Interface target can be used, see above.
#    add_library(${PROJECT_NAME} SHARED
#        ${${PROJECT_NAME}_lib_hdr_build}
#        ${${PROJECT_NAME}_lib_tpp_build}
#        ${${PROJECT_NAME}_lib_qt_hdr_build}
#        ${${PROJECT_NAME}_lib_src_build}
#        ${${PROJECT_NAME}_lib_hdr_moc_build})
#    target_sources(${PROJECT_NAME} PRIVATE
#        $<TARGET_PROPERTY:${${PROJECT_NAME}_private_target},INTERFACE_SOURCES>)
##        $<TARGET_PROPERTY:${${PROJECT_NAME}_private_target},qt_hdr>
##        $<TARGET_PROPERTY:${${PROJECT_NAME}_private_target},qt_ui>)
#    target_link_libraries(${PROJECT_NAME}
#        Qt5::Core Qt5::SerialPort
#        Threads::Threads
#        lumik::enum_flags::enum_flags
#        biomolecules::sprelay::sprelay_globals
#        biomolecules::sprelay::${${PROJECT_NAME}_private_target})
    # create the library
    add_library(${PROJECT_NAME} SHARED
        ${${PROJECT_NAME}_lib_src_build}
        ${${PROJECT_NAME}_lib_hdr_moc_build}
        ${${PROJECT_NAME}_src_build}
        ${${PROJECT_NAME}_hdr_moc_build}
        ${${PROJECT_NAME}_ui_moc_build})
    target_link_libraries(${PROJECT_NAME}
        Qt5::Core
        Qt5::SerialPort
        Threads::Threads
        lumik::enum_flags::enum_flags
        biomolecules::sprelay::sprelay_globals)

    # attach header files to the library (mainly to display them in IDEs)
    target_sources(${PROJECT_NAME}
        PUBLIC
            $<INSTALL_INTERFACE:${${PROJECT_NAME}_lib_hdr_install}>
            $<BUILD_INTERFACE:${${PROJECT_NAME}_lib_hdr_build}>
            $<INSTALL_INTERFACE:${${PROJECT_NAME}_lib_tpp_install}>
            $<BUILD_INTERFACE:${${PROJECT_NAME}_lib_tpp_build}>
            $<INSTALL_INTERFACE:${${PROJECT_NAME}_lib_qt_hdr_install}>
            $<BUILD_INTERFACE:${${PROJECT_NAME}_lib_qt_hdr_build}>
        PRIVATE
            $<BUILD_INTERFACE:${${PROJECT_NAME}_hdr_build}>
            $<BUILD_INTERFACE:${${PROJECT_NAME}_tpp_build}>
            $<BUILD_INTERFACE:${${PROJECT_NAME}_qt_hdr_build}>)
endif()

# finish the library creation
target_include_directories(${PROJECT_NAME} PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${sprelay_source_dir}>)
target_compile_definitions(${PROJECT_NAME} PRIVATE SPRELAY_LIBRARY)

# create alias to enable treating the library inside this project as if it were imported in namespace
add_library(biomolecules::${sprelay_project_name}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# install the library and export it to target list
install(TARGETS ${PROJECT_NAME} EXPORT ${sprelay_project_name}Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
if (NOT BUILD_STANDALONE)
    install(FILES ${${PROJECT_NAME}_lib_hdr_build} ${${PROJECT_NAME}_lib_qt_hdr_build}
        DESTINATION ${sprelay_core_install_include_dir})
endif()

if (ENABLE_COVERAGE)
    target_link_libraries(${PROJECT_NAME} -fprofile-instr-generate -fcoverage-mapping)
    set_property(GLOBAL APPEND PROPERTY coverage_binaries ${PROJECT_NAME})
endif()

# Documentation sources - Qt creator does not support interface libraries so the documentation sources are added to
# normal targets
if (DOXYGEN_FOUND)
    set(${PROJECT_NAME}_doc_src
        ${PROJECT_SOURCE_DIR}/core.dox
        ${PROJECT_SOURCE_DIR}/k8090.dox
        ${PROJECT_SOURCE_DIR}/serial_port.dox)
    target_sources(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_doc_src})
    set_source_files_properties(${${PROJECT_NAME}_doc_src} PROPERTIES HEADER_FILE_ONLY TRUE)
endif()
