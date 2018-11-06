project(${sprelay_project_name}_core_test)

# collect files

# tests
set(${PROJECT_NAME}_hdr
    ${PROJECT_SOURCE_DIR}/core_test_utils.h)
set(${PROJECT_NAME}_tpp)
set(${PROJECT_NAME}_qt_hdr
    ${PROJECT_SOURCE_DIR}/command_queue_test.h
    ${PROJECT_SOURCE_DIR}/k8090_test.h
    ${PROJECT_SOURCE_DIR}/k8090_utils_test.h
    ${PROJECT_SOURCE_DIR}/mock_serial_port_test.h
    ${PROJECT_SOURCE_DIR}/serial_port_utils_test.h
    ${PROJECT_SOURCE_DIR}/unified_serial_port_test.h)
set(${PROJECT_NAME}_src
    ${PROJECT_SOURCE_DIR}/command_queue_test.cpp
    ${PROJECT_SOURCE_DIR}/core_test.cpp
    ${PROJECT_SOURCE_DIR}/k8090_test.cpp
    ${PROJECT_SOURCE_DIR}/k8090_utils_test.cpp
    ${PROJECT_SOURCE_DIR}/mock_serial_port_test.cpp
    ${PROJECT_SOURCE_DIR}/serial_port_utils_test.cpp
    ${PROJECT_SOURCE_DIR}/unified_serial_port_test.cpp)
set(${PROJECT_NAME}_ui)

if (NOT use_object_targets)
    # tested
    set(sprelay_core_private sprelay_core_private)
    set(sprelay_core_source_dir "${sprelay_root_source_dir}/src/biomolecules/sprelay/core")

    set(${sprelay_core_private}_hdr
        ${sprelay_core_source_dir}/command_queue.h
        ${sprelay_core_source_dir}/k8090_commands.h
        ${sprelay_core_source_dir}/k8090_utils.h
        ${sprelay_core_source_dir}/serial_port_utils.h)
    set(${sprelay_core_private}_tpp
        ${sprelay_core_source_dir}/command_queue.tpp)
    set(${sprelay_core_private}_qt_hdr
        ${sprelay_core_source_dir}/mock_serial_port.h
        ${sprelay_core_source_dir}/unified_serial_port.h)
    set(${sprelay_core_private}_src
        ${sprelay_core_source_dir}/k8090_utils.cpp
        ${sprelay_core_source_dir}/mock_serial_port.cpp
        ${sprelay_core_source_dir}/serial_port_utils.cpp
        ${sprelay_core_source_dir}/unified_serial_port.cpp)
endif()

# call qt moc
qt5_wrap_cpp(${PROJECT_NAME}_hdr_moc ${${PROJECT_NAME}_qt_hdr})
qt5_wrap_ui(${PROJECT_NAME}_ui_moc ${${PROJECT_NAME}_ui})
if (NOT use_object_targets)
    qt5_wrap_cpp(${sprelay_core_private}_hdr_moc ${${sprelay_core_private}_qt_hdr})
    # TODO(lumik): thing of better way. The following is dangerous because it gets property during from the
    # sprelay_core_private during CMakeLists parsing
#    get_property(${sprelay_core_private}_qt_hdr TARGET sprelay_core_private PROPERTY qt_hdr)
#    qt5_wrap_cpp(${sprelay_core_private}_hdr_moc ${${sprelay_core_private}_qt_hdr})
    # The following cant be done because you can't use generator expressions in add_custom_command as of cmake 3.12 and
    # qt5_wrap_cpp uses add_custom_command.
#    qt5_wrap_cpp(${sprelay_core_private}_hdr_moc $<TARGET_PROPERTY:sprelay_private_target,qt_hdr>)
endif()


# core test #
# --------- #

if (use_object_targets)
    add_executable(${PROJECT_NAME}
        ${${PROJECT_NAME}_src}
        ${${PROJECT_NAME}_hdr_moc}
        ${${PROJECT_NAME}_ui_moc})
    target_link_libraries(${PROJECT_NAME}
        Qt5::Core
        Qt5::Test
        Threads::Threads
        qtest_suite
        biomolecules::sprelay::sprelay_core
        biomolecules::sprelay::sprelay_core_private)
    target_include_directories(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:${sprelay_tests_source_dir}>)

    # attach header files to the library (mainly to display them in IDEs)
    target_sources(${PROJECT_NAME} PRIVATE
        ${${PROJECT_NAME}_hdr}
        ${${PROJECT_NAME}_tpp}
        ${${PROJECT_NAME}_qt_hdr})
else()
#    # TODO(lumik): the following is dangerous due to moc files generation (see above) so it is not used
#    add_executable(${PROJECT_NAME}
#        ${${PROJECT_NAME}_hdr}
#        ${${PROJECT_NAME}_tpp}
#        ${${PROJECT_NAME}_qt_hdr}
#        ${${PROJECT_NAME}_src}
#        ${${PROJECT_NAME}_hdr_moc}
#        ${${PROJECT_NAME}_ui_moc})
#    target_sources(${PROJECT_NAME} PRIVATE $<TARGET_PROPERTY:sprelay_core_private,INTERFACE_SOURCES>)
#    target_link_libraries(${PROJECT_NAME}
#        Qt5::Core
#        Qt5::Test
#        Threads::Threads
#        qtest_suite
#        sprelay_core
#        sprelay_core_private)
#    target_include_directories(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:${sprelay_tests_source_dir}>)
    add_executable(${PROJECT_NAME}
        ${${PROJECT_NAME}_src}
        ${${PROJECT_NAME}_hdr_moc}
        ${${PROJECT_NAME}_ui_moc}
        ${${sprelay_core_private}_src}
        ${${sprelay_core_private}_hdr_moc})
    target_link_libraries(${PROJECT_NAME}
        Qt5::Core
        Qt5::SerialPort
        Qt5::Test
        lumik::enum_flags::enum_flags
        qtest_suite
        biomolecules::sprelay::sprelay_globals
        biomolecules::sprelay::sprelay_core)
    target_include_directories(${PROJECT_NAME}
        PRIVATE
            $<BUILD_INTERFACE:${sprelay_tests_source_dir}>
            $<BUILD_INTERFACE:${sprelay_root_source_dir}/src>)

    # attach header files to the library (mainly to display them in IDEs)
    target_sources(${PROJECT_NAME} PRIVATE
        ${${PROJECT_NAME}_hdr}
        ${${PROJECT_NAME}_tpp}
        ${${PROJECT_NAME}_qt_hdr}
        ${${sprelay_core_private}_hdr}
        ${${sprelay_core_private}_tpp}
        ${${sprelay_core_private}_qt_hdr})
endif()

if (sprelay_standalone_console_link_flags)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS ${sprelay_standalone_console_link_flags})
endif()

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME} -silent)
