cmake_minimum_required(VERSION 3.1.3)
project(sprelay VERSION 1.0.0 LANGUAGES CXX)

option(BUILD_STANDALONE
    "Build the project as a standalone application."
    OFF)

option(SKIP_GUI
    "Build the without GUI. Takes effect only if BUILD_STANDALONE=OFF"
    OFF)
if (BUILD_STANDALONE AND SKIP_GUI)
    message(WARNING "SKIP_GUI does not take any effect in combination with BUILD_STANDALONE.")
endif()

option(MAKE_ENUM_FLAGS
    "Forces internal enum_flags library usage. Otherwise the library is searched if sytem path. You can also specify
    its location by enum_flags_ROOT_DIR cache variable."
    OFF)

if (NOT MAKE_ENUM_FLAGS)
    set(enum_flags_ROOT_DIR "" CACHE PATH "Location hint for enum_flags library search.")
endif()

set(CPPREFERENCE_TAGS_ROOT_DIR "" CACHE PATH "Location hint for the cppreference doxygen tags search.")

option(MAKE_TESTS
    "Makes tests."
    OFF)

# set some globals
set(sprelay_project_name sprelay)
set(sprelay_root_binary_dir ${CMAKE_BINARY_DIR})
set(sprelay_root_source_dir ${PROJECT_SOURCE_DIR})
set(sprelay_install_include_dir "${CMAKE_INSTALL_INCLUDEDIR}/biomolecules/${sprelay_project_name}")
set(sprelay_install_cmake_dir "${CMAKE_INSTALL_DATAROOTDIR}/cmake/${sprelay_project_name}")
set(sprelay_bin_cmake_dir "${sprelay_root_binary_dir}/${CMAKE_INSTALL_DATAROOTDIR}/cmake/${sprelay_project_name}")
set(sprelay_install_doc_dir "${CMAKE_INSTALL_DATAROOTDIR}/doc/${sprelay_project_name}")

# object files with dependencies works well only for cmake version 3.12 or later else the object files do not propagate
# dependencies and cant use target_link_libraries
if (NOT CMAKE_VERSION VERSION_LESS "3.12")
    set(use_object_targets TRUE)
endif()

# set cmake install prefix default on windows
if (CMAKE_INSTALL_PREFIX MATCHES "^C:/Program Files")
    # Override to an "Install" directory in the current build-dir
    message(STATUS "    Install-prefix was at default -> forcing it to the ${PROJECT_NAME} directory:")
    message(STATUS "        'CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX})'")
    set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
endif()

# setup compiler flags
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-pipe -frtti -Wall -Wextra -Wpedantic -fexceptions)
endif()

if (MSVC)
    add_compile_options(/W4)
endif()

if (MINGW)
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

# check for Doxygen
find_package(Doxygen)

# check for python for pylint
find_package(PythonInterp)

# install correct libraries into correct locations on all platforms
include(GNUInstallDirs)

# enable running program from build directory by copying all binary staff into bin subdirectory
if (WIN32)
    if (NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${sprelay_root_binary_dir}/${CMAKE_INSTALL_LIBDIR}")
    endif()
    if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${sprelay_root_binary_dir}/${CMAKE_INSTALL_LIBDIR}")
    endif()
    if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${sprelay_root_binary_dir}/${CMAKE_INSTALL_BINDIR}")
    endif()
endif()

if (NOT BUILD_STANDALONE)
    # create package to be discoverable in find package
    include(CMakePackageConfigHelpers)
    configure_package_config_file(cmake/${sprelay_project_name}Config.cmake.in
        "${sprelay_bin_cmake_dir}/${sprelay_project_name}Config.cmake"
        INSTALL_DESTINATION "${sprelay_install_cmake_dir}"
        NO_SET_AND_CHECK_MACRO
        NO_CHECK_REQUIRED_COMPONENTS_MACRO)
    write_basic_package_version_file(
        "${sprelay_bin_cmake_dir}/${sprelay_project_name}ConfigVersion.cmake"
        COMPATIBILITY SameMajorVersion)
    install(
        FILES
            "${sprelay_bin_cmake_dir}/${sprelay_project_name}Config.cmake"
            "${sprelay_bin_cmake_dir}/${sprelay_project_name}ConfigVersion.cmake"
        DESTINATION "${sprelay_install_cmake_dir}"
        COMPONENT Develop)
endif()

# execute all parts of projects placed in subdirectories
include(src/CMakeLists.txt)
if (MAKE_TESTS)
    add_subdirectory(tests)
endif()

# documentation
if (DOXYGEN_FOUND)
    # find cppreference file
    find_file(cppreference_tag_file cppreference-doxygen-web.tag.xml
        HINTS
            ${CPPREFERENCE_TAGS_ROOT_DIR}
            $ENV{CPPREFERENCE_TAGS_ROOT_DIR})
    if (cppreference_tag_file)
        set(cppreference_tags "\"${cppreference_tag_file}=http://en.cppreference.com/w/\"")
    else()
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/cppreference-doxygen-web.tag.xml
            COMMAND ${CMAKE_COMMAND}
                -D DEST_DIR=${CMAKE_CURRENT_BINARY_DIR}
                -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/get_cppreference_tags.cmake
            COMMENT "Getting cppreference.com doxygen tags...")
        add_custom_target(get_ccpreference_tags DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/cppreference-doxygen-web.tag.xml)
        set(cppreference_tags
            "\"${CMAKE_CURRENT_BINARY_DIR}/cppreference-doxygen-web.tag.xml=http://en.cppreference.com/w/\"")
    endif()

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

    # enum_flags documentation
    get_target_property(enum_flags_tags enum_flags_tag_file TAG_FILE)
    get_target_property(enum_flags_doc_dir enum_flags_tag_file DOC_DIR)
    if (NOT "${enum_flags_doc_dir}" STREQUAL "")
        set(enum_flags_tags "${enum_flags_tags}=${enum_flags_doc_dir}")
    elseif (NOT "${enum_flags_tags}" STREQUAL "")
        set(enum_flags_tags "${enum_flags_tags}=../../${CMAKE_INSTALL_DATAROOTDIR}/doc/enum_flags/html/")
    else()
        set(enum_flags_tags "")
    endif()
    get_filename_component(sprelay_parent_dir ${sprelay_root_source_dir} DIRECTORY)

    # configure doxygen
    set(sprelay_tags ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.tags)
    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    set(doxy_main_page ${CMAKE_CURRENT_SOURCE_DIR}/src/biomolecules/sprelay/index.md)
    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating documentation with Doxygen"
        VERBATIM)
    add_dependencies(doc enum_flags_tag_file)
    if (NOT cppreference_tag_file)
        add_dependencies(doc get_ccpreference_tags)
    endif()

    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/html DESTINATION ${sprelay_install_doc_dir} OPTIONAL)
    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.tags
        DESTINATION ${sprelay_install_doc_dir} OPTIONAL)
endif()

# Other tests
if (PYTHONINTERP_FOUND)
    add_test(NAME cpplint
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${PYTHON_EXECUTABLE}
            external/cpplint-1.3.0/cpplint.py
            --extensions=cpp,h,tpp --headers=h,tpp --recursive src/biomolecules/sprelay tests/biomolecules/sprelay)
endif()
