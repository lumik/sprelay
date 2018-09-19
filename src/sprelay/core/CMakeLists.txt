project(${sprelay_project_name}_core)

# collect files
set(${PROJECT_NAME}_lib_hdr
    ${PROJECT_SOURCE_DIR}/k8090_defines.h
    ${PROJECT_SOURCE_DIR}/serial_port_defines.h)
set(${PROJECT_NAME}_lib_qt_hdr
    ${PROJECT_SOURCE_DIR}/k8090.h)
set(${PROJECT_NAME}_lib_tpp)
set(${PROJECT_NAME}_lib_src
    ${PROJECT_SOURCE_DIR}/k8090.cpp)
set(${PROJECT_NAME}_hdr
    ${PROJECT_SOURCE_DIR}/command_queue.h
    ${PROJECT_SOURCE_DIR}/concurent_command_queue.h
    ${PROJECT_SOURCE_DIR}/k8090_commands.h
    ${PROJECT_SOURCE_DIR}/k8090_utils.h
    ${PROJECT_SOURCE_DIR}/serial_port_utils.h)
set(${PROJECT_NAME}_qt_hdr
    ${PROJECT_SOURCE_DIR}/mock_serial_port.h
    ${PROJECT_SOURCE_DIR}/unified_serial_port.h)
set(${PROJECT_NAME}_tpp
    ${PROJECT_SOURCE_DIR}/command_queue.tpp)
set(${PROJECT_NAME}_src
    ${PROJECT_SOURCE_DIR}/concurent_command_queue.cpp
    ${PROJECT_SOURCE_DIR}/k8090_utils.cpp
    ${PROJECT_SOURCE_DIR}/mock_serial_port.cpp
    ${PROJECT_SOURCE_DIR}/serial_port_utils.cpp
    ${PROJECT_SOURCE_DIR}/unified_serial_port.cpp)
set(${PROJECT_NAME}_ui)

# call qt moc
qt5_wrap_cpp(${PROJECT_NAME}_hdr_moc ${${PROJECT_NAME}_qt_hdr})
qt5_wrap_cpp(${PROJECT_NAME}_lib_hdr_moc ${${PROJECT_NAME}_lib_qt_hdr})
qt5_wrap_ui(${PROJECT_NAME}_ui_moc ${${PROJECT_NAME}_ui})


# object files - can be used in tests #
# ----------------------------------- #

if(use_object_targets)
    set(${PROJECT_NAME}_private_target ${PROJECT_NAME}_private)

    add_library(${${PROJECT_NAME}_private_target} OBJECT
        ${${PROJECT_NAME}_lib_hdr}
        ${${PROJECT_NAME}_lib_tpp}
        ${${PROJECT_NAME}_lib_qt_hdr}
        ${${PROJECT_NAME}_hdr}
        ${${PROJECT_NAME}_tpp}
        ${${PROJECT_NAME}_qt_hdr}
        ${${PROJECT_NAME}_src}
        ${${PROJECT_NAME}_hdr_moc}
        ${${PROJECT_NAME}_ui_moc})
    target_link_libraries(${${PROJECT_NAME}_private_target}
        Qt5::Core
        Qt5::SerialPort
        Threads::Threads
        enum_flags
        sprelay_global)
    target_include_directories(${${PROJECT_NAME}_private_target} SYSTEM PUBLIC $<INSTALL_INTERFACE:include>)
    target_include_directories(${${PROJECT_NAME}_private_target} PUBLIC $<BUILD_INTERFACE:${sprelay_source_dir}>)

    if(NOT SPRELAY_BUILD_STANDALONE)
        install(TARGETS ${${PROJECT_NAME}_private_target} EXPORT ${${PROJECT_NAME}_private_target}Config
            # ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
        install(EXPORT ${${PROJECT_NAME}_private_target}Config DESTINATION share/${${PROJECT_NAME}_private_target}/cmake)
        export(TARGETS ${${PROJECT_NAME}_private_target} FILE ${${PROJECT_NAME}_private_target}Config.cmake)
    endif(NOT SPRELAY_BUILD_STANDALONE)
# TODO(lumik): interface targets can't be used as the replacement for object files for older cmake versions because
# problems with qt moc, see comments in tests. Devise better approach to define used headers here.
#else()
#    set(${PROJECT_NAME}_private_target ${PROJECT_NAME}_private)

#    add_library(${${PROJECT_NAME}_private_target} INTERFACE)
##    set_property(TARGET ${${PROJECT_NAME}_private_target} PROPERTY qt_hdr ${${PROJECT_NAME}_qt_hdr})
##    set_property(TARGET ${${PROJECT_NAME}_private_target} PROPERTY qt_ui ${${PROJECT_NAME}_ui})
#    target_sources(${${PROJECT_NAME}_private_target} INTERFACE
#        ${${PROJECT_NAME}_lib_hdr}
#        ${${PROJECT_NAME}_lib_tpp}
#        ${${PROJECT_NAME}_lib_qt_hdr}
#        ${${PROJECT_NAME}_hdr}
#        ${${PROJECT_NAME}_tpp}
#        ${${PROJECT_NAME}_qt_hdr}
#        ${${PROJECT_NAME}_src}
#        ${${PROJECT_NAME}_hdr_moc}
#        ${${PROJECT_NAME}_ui_moc})
#    target_link_libraries(${${PROJECT_NAME}_private_target} INTERFACE
#        Qt5::Core
#        Qt5::SerialPort
#        Threads::Threads
#        enum_flags
#        sprelay_global)
#    target_include_directories(${${PROJECT_NAME}_private_target} SYSTEM INTERFACE $<INSTALL_INTERFACE:include>)
#    target_include_directories(${${PROJECT_NAME}_private_target} INTERFACE $<BUILD_INTERFACE:${sprelay_source_dir}>)

#    if(NOT SPRELAY_BUILD_STANDALONE)
#        install(TARGETS ${${PROJECT_NAME}_private_target} EXPORT ${${PROJECT_NAME}_private_target}Config
#            # ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
#        install(EXPORT ${${PROJECT_NAME}_private_target}Config DESTINATION share/${${PROJECT_NAME}_private_target}/cmake)
#        export(TARGETS ${${PROJECT_NAME}_private_target} FILE ${${PROJECT_NAME}_private_target}Config.cmake)
#    endif(NOT SPRELAY_BUILD_STANDALONE)
endif(use_object_targets)


# core library #
# ------------ #

if(use_object_targets)
    add_library(${PROJECT_NAME} SHARED
        ${${PROJECT_NAME}_lib_hdr}
        ${${PROJECT_NAME}_lib_tpp}
        ${${PROJECT_NAME}_lib_qt_hdr}
        ${${PROJECT_NAME}_lib_src}
        ${${PROJECT_NAME}_lib_hdr_moc})
    target_link_libraries(${PROJECT_NAME}
        Qt5::Core Qt5::SerialPort
        Threads::Threads
        enum_flags
        sprelay_global
        ${${PROJECT_NAME}_private_target})
else()
#    # Interface target can be used, see above.
#    add_library(${PROJECT_NAME} SHARED
#        ${${PROJECT_NAME}_lib_hdr}
#        ${${PROJECT_NAME}_lib_tpp}
#        ${${PROJECT_NAME}_lib_qt_hdr}
#        ${${PROJECT_NAME}_lib_src}
#        ${${PROJECT_NAME}_lib_hdr_moc})
#    target_sources(${PROJECT_NAME} PRIVATE
#        $<TARGET_PROPERTY:${${PROJECT_NAME}_private_target},INTERFACE_SOURCES>)
##        $<TARGET_PROPERTY:${${PROJECT_NAME}_private_target},qt_hdr>
##        $<TARGET_PROPERTY:${${PROJECT_NAME}_private_target},qt_ui>)
#    target_link_libraries(${PROJECT_NAME}
#        Qt5::Core Qt5::SerialPort
#        Threads::Threads
#        enum_flags
#        sprelay_global
#        ${${PROJECT_NAME}_private_target})
    add_library(${PROJECT_NAME} SHARED
        ${${PROJECT_NAME}_lib_hdr}
        ${${PROJECT_NAME}_lib_tpp}
        ${${PROJECT_NAME}_lib_qt_hdr}
        ${${PROJECT_NAME}_lib_src}
        ${${PROJECT_NAME}_lib_hdr_moc}
        ${${PROJECT_NAME}_hdr}
        ${${PROJECT_NAME}_tpp}
        ${${PROJECT_NAME}_qt_hdr}
        ${${PROJECT_NAME}_src}
        ${${PROJECT_NAME}_hdr_moc}
        ${${PROJECT_NAME}_ui_moc})
    target_link_libraries(${PROJECT_NAME}
        Qt5::Core
        Qt5::SerialPort
        Threads::Threads
        enum_flags
        sprelay_global)
endif(use_object_targets)

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${sprelay_source_dir}>)
target_compile_definitions(${PROJECT_NAME} PRIVATE SPRELAY_LIBRARY)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Config
    # ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
if(NOT SPRELAY_BUILD_STANDALONE)
    install(EXPORT ${PROJECT_NAME}Config DESTINATION share/${PROJECT_NAME}/cmake)
    export(TARGETS ${PROJECT_NAME} FILE ${PROJECT_NAME}Config.cmake)

    install(FILES ${${PROJECT_NAME}_lib_hdr} ${${PROJECT_NAME}_lib_qt_hdr}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sprelay/core)
endif(NOT SPRELAY_BUILD_STANDALONE)

# Documentation sources - Qt creator does not support interface libraries so the documentation sources are added to
# normal targets
if(DOXYGEN_FOUND)
    set(${PROJECT_NAME}_doc_src
        ${PROJECT_SOURCE_DIR}/core.dox
        ${PROJECT_SOURCE_DIR}/k8090.dox
        ${PROJECT_SOURCE_DIR}/serial_port.dox)
    target_sources(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_doc_src})
    set_source_files_properties(${${PROJECT_NAME}_doc_src} PROPERTIES HEADER_FILE_ONLY TRUE)
endif(DOXYGEN_FOUND)
