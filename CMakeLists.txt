cmake_minimum_required(VERSION 3.1.3)
project(sprelay	VERSION 1.0.0 LANGUAGES CXX)

OPTION(SPRELAY_BUILD_STANDALONE
    "Build the project as a standalone application."
    OFF)

OPTION(SPRELAY_SKIP_GUI
    "Build the without GUI. Takes effect only if SPRELAY_BUILD_STANDALONE=OFF"
    OFF)
if(SPRELAY_BUILD_STANDALONE AND SPRELAY_SKIP_GUI)
    message(WARNING "SPRELAY_SKIP_GUI does not take any effect in combination with SPRELAY_BUILD_STANDALONE.")
endif(SPRELAY_BUILD_STANDALONE AND SPRELAY_SKIP_GUI)

OPTION(SPRELAY_INSTALL_ENUM_FLAGS
    "Installs required dependece enum_flags with the library."
    OFF)
if(SPRELAY_BUILD_STANDALONE AND INSTALL_SPRELAY_ENUM_FLAGS)
    message(WARNING "INSTALL_SPRELAY_ENUM_FLAGS does not take any effect in combination with SPRELAY_BUILD_STANDALONE.")
endif(SPRELAY_BUILD_STANDALONE AND INSTALL_SPRELAY_ENUM_FLAGS)

OPTION(SPRELAY_MAKE_TESTS
    "Makes tests."
    OFF)

# set some globals
set(sprelay_project_name sprelay)
set(sprelay_global_binary_dir ${CMAKE_BINARY_DIR})
set(sprelay_global_source_dir ${PROJECT_SOURCE_DIR})
# object files with dependencies works well only for cmake version 3.12 or later else the object files do not propagate
# dependencies and cant use target_link_libraries
if(NOT CMAKE_VERSION VERSION_LESS "3.12")
    set(use_object_targets TRUE)
endif(NOT CMAKE_VERSION VERSION_LESS "3.12")

# set cmake install prefix default on windows
if (CMAKE_INSTALL_PREFIX MATCHES "^C:/Program Files")
    # Override to an "Install" directory in the current build-dir
    message(STATUS "    Install-prefix was at default -> forcing it to the ${PROJECT_NAME} directory:")
    message(STATUS "        'CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX})'")
    set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
endif(CMAKE_INSTALL_PREFIX MATCHES "^C:/Program Files")

# setup compiler flags
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-pipe -frtti -Wall -Wextra -fexceptions)
endif()
if(MINGW)
    add_compile_options(-mthreads -fno-keep-inline-dllexport -DUNICODE)
    list(APPEND sprelay_standalone_application_link_flags "-Wl,-subsystem,windows")
    list(APPEND sprelay_standalone_console_link_flags "-Wl,-subsystem,console")
endif()

# enable tests
enable_testing()

# Add Qt
find_package(Qt5 COMPONENTS Core Gui Widgets SerialPort Test REQUIRED)

# Add threading
#set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
#set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# install correct libraries into correct locations on all platforms
include(GNUInstallDirs)

# enable running program from build directory by copying all binary staff into bin subdirectory
if(WIN32)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${sprelay_global_binary_dir}/${CMAKE_INSTALL_LIBDIR}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${sprelay_global_binary_dir}/${CMAKE_INSTALL_LIBDIR}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${sprelay_global_binary_dir}/${CMAKE_INSTALL_BINDIR}")
endif()

# check for Doxygen
find_package(Doxygen)

# check for python for pylint
find_package(PythonInterp)


# execute all parts of projects placed in subdirectories
include(src/CMakeLists.txt)
if(SPRELAY_MAKE_TESTS)
    add_subdirectory(tests)
endif(SPRELAY_MAKE_TESTS)

# documentation
if(DOXYGEN_FOUND)
    # get Qt documentation location
    get_target_property(qmake_executable Qt5::qmake LOCATION)
    execute_process(
        COMMAND ${qmake_executable} -query QT_INSTALL_DOCS
        RESULT_VARIABLE return_code
        OUTPUT_VARIABLE qt_docs_dir
    )

    # link in qts own documentation
    string(STRIP ${qt_docs_dir} qt_docs_dir)
    set(qt_core_tags "${qt_docs_dir}/qtcore/qtcore.tags=http://doc.qt.io/qt-5/")
    set(qt_gui_tags "${qt_docs_dir}/qtgui/qtgui.tags=http://doc.qt.io/qt-5/")
    set(qt_widgets_tags "${qt_docs_dir}/qtwidgets/qtwidgets.tags=http://doc.qt.io/qt-5/")
    set(qt_testlib_tags "${qt_docs_dir}/qttestlib/qttestlib.tags=http://doc.qt.io/qt-5/")
    get_target_property(enum_flags_tags enum_flags_tag_file TAG_FILE)
    set(enum_flags_tags "${enum_flags_tags}=../../${CMAKE_INSTALL_DATAROOTDIR}/doc/enum_flags/html/")

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    set(doxy_main_page ${CMAKE_CURRENT_SOURCE_DIR}/src/sprelay/index.md)
    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating documentation with Doxygen"
        VERBATIM)
    add_dependencies(doc enum_flags_tag_file)
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc DESTINATION ${CMAKE_INSTALL_PREFIX} OPTIONAL)
endif(DOXYGEN_FOUND)

# Other tests
if(PYTHONINTERP_FOUND)
    add_test(NAME pylint
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${PYTHON_EXECUTABLE}
            third_party/cpplint-1.3.0/cpplint.py
            --extensions=cpp,h,tpp --headers=h,tpp --recursive src/sprelay tests/sprelay)
endif(PYTHONINTERP_FOUND)
