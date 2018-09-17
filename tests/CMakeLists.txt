# save sprelay_tests_source_dir
set(sprelay_tests_source_dir ${CMAKE_CURRENT_SOURCE_DIR})


# qtest_suite #
# ----------- #

# import qtest_suite
# TODO(lumik): devise more general way to do that
set(qtest_suite_target qtest_suite)
add_library(${qtest_suite_target} INTERFACE)
target_include_directories(${qtest_suite_target}
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/tests/external/qtest_suite/include>)

# build tests
add_subdirectory(sprelay)
