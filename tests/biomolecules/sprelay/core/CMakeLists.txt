project(${sprelay_project_name}_core_test)

# collect files

# tests
set(${PROJECT_NAME}_hdr
    ${PROJECT_SOURCE_DIR}/impl/core_test_utils.h)
set(${PROJECT_NAME}_tpp)
set(${PROJECT_NAME}_qt_hdr
    ${PROJECT_SOURCE_DIR}/k8090_test.h)
set(${PROJECT_NAME}_src
    ${PROJECT_SOURCE_DIR}/core_test.cpp
    ${PROJECT_SOURCE_DIR}/k8090_test.cpp)
set(${PROJECT_NAME}_ui)

# call qt moc
qt5_wrap_cpp(${PROJECT_NAME}_hdr_moc ${${PROJECT_NAME}_qt_hdr})
qt5_wrap_ui(${PROJECT_NAME}_ui_moc ${${PROJECT_NAME}_ui})


# core test #
# --------- #

add_executable(${PROJECT_NAME}
    ${${PROJECT_NAME}_src}
    ${${PROJECT_NAME}_hdr_moc}
    ${${PROJECT_NAME}_ui_moc})
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Test
    Threads::Threads
    qtest_suite
    biomolecules::sprelay::sprelay_core)
target_include_directories(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:${sprelay_tests_source_dir}>)

# attach header files to the library (mainly to display them in IDEs)
target_sources(${PROJECT_NAME} PRIVATE
    ${${PROJECT_NAME}_hdr}
    ${${PROJECT_NAME}_tpp}
    ${${PROJECT_NAME}_qt_hdr})

if (sprelay_standalone_console_link_flags)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS ${sprelay_standalone_console_link_flags})
endif()

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME} -silent)

if (ENABLE_COVERAGE)
    target_link_libraries(${PROJECT_NAME} -fprofile-instr-generate -fcoverage-mapping)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.profraw
        COMMAND LLVM_PROFILE_FILE=${PROJECT_NAME}.profraw ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} -silent
        DEPENDS ${PROJECT_NAME}
        COMMENT "${PROJECT_NAME}: Creating raw coverage data...")
    add_custom_target(${PROJECT_NAME}_coverage
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.profraw)
    set_property(GLOBAL APPEND PROPERTY coverage_raw_files "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.profraw")
    set_property(GLOBAL APPEND PROPERTY coverage_binaries ${PROJECT_NAME})
    set_property(GLOBAL APPEND PROPERTY coverage_targets ${PROJECT_NAME}_coverage)
endif()

# link in sanitizers
if (ADDRESS_SANITIZE)
    target_link_libraries(${PROJECT_NAME} -fsanitize=address)
    set_tests_properties(${PROJECT_NAME} PROPERTIES ENVIRONMENT ASAN_OPTIONS=verbosity=1:detect_leaks=1:check_initialization_order=1)
endif()
if (THREAD_SANITIZE)
    target_link_libraries(${PROJECT_NAME} -fsanitize=thread)
    set_tests_properties(${PROJECT_NAME} PROPERTIES ENVIRONMENT TSAN_OPTIONS=verbosity=1)
endif()
if (UB_SANITIZE)
    target_link_libraries(${PROJECT_NAME} -fsanitize=undefined)
    set_tests_properties(${PROJECT_NAME} PROPERTIES ENVIRONMENT UBSAN_OPTIONS=verbosity=1)
endif()

add_subdirectory(impl)
