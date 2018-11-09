project(${sprelay_project_name}_src)

# sprelay globals #
# --------------- #

# collect library files
set(sprelay_globals_name ${sprelay_project_name}_globals)
set(${sprelay_globals_name}_lib_hdr
    sprelay_global.h)

# create build and install file paths
foreach(hdr ${${sprelay_globals_name}_lib_hdr})
    list(APPEND ${sprelay_globals_name}_lib_hdr_build "${PROJECT_SOURCE_DIR}/${hdr}")
    list(APPEND ${sprelay_globals_name}_lib_hdr_install
        "${sprelay_install_include_dir}/${hdr}")
endforeach()

# create the library
add_library(${sprelay_globals_name} INTERFACE)
target_include_directories(${sprelay_globals_name} INTERFACE
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${sprelay_source_dir}>)

# attach header files to the library (mainly to display them in IDEs)
target_sources(${sprelay_globals_name} INTERFACE
    $<INSTALL_INTERFACE:${${sprelay_globals_name}_lib_hdr_install}>
    $<BUILD_INTERFACE:${${sprelay_globals_name}_lib_hdr_build}>)

# create alias to enable treating the library inside this project as if it were imported in namespace
add_library(biomolecules::${sprelay_project_name}::${sprelay_globals_name} ALIAS ${sprelay_globals_name})

if (NOT BUILD_STANDALONE)
    # install the library and export it to target list
    install(FILES ${${sprelay_globals_name}_lib_hdr_build}
        DESTINATION "${sprelay_install_include_dir}")
    install(TARGETS ${sprelay_globals_name} EXPORT ${sprelay_project_name}Targets
        PUBLIC_HEADER DESTINATION ${sprelay_install_include_dir})
endif()


# sprelay gui #
# ----------- #

if (BUILD_STANDALONE OR NOT SKIP_GUI)
    # collect files
    set(${sprelay_project_name}_src "")

    # compile files connected only with standalone application on demand
    if (BUILD_STANDALONE)
        list(APPEND ${sprelay_project_name}_src
            ${PROJECT_SOURCE_DIR}/main.cpp)
    endif()

    # create executable or dll
    if (BUILD_STANDALONE)
        # create the executable
        add_executable(${sprelay_project_name}
            ${${sprelay_project_name}_src})
        target_compile_definitions(${sprelay_project_name} PRIVATE SPRELAY_STANDALONE)

        # add possible link flags
        if (sprelay_standalone_application_link_flags)
            set_target_properties(${sprelay_project_name} PROPERTIES LINK_FLAGS
                ${sprelay_standalone_application_link_flags})
        endif()
    else()
        # create the library
        add_library(${sprelay_project_name} SHARED
            ${${sprelay_project_name}_src})
        target_compile_definitions(${sprelay_project_name} PRIVATE SPRELAY_MAIN_LIBRARY)
    endif()

    target_link_libraries(${sprelay_project_name}
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Threads::Threads
        biomolecules::sprelay::sprelay_globals)
    set_target_properties(${sprelay_project_name} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
    set_target_properties(${sprelay_project_name} PROPERTIES INSTALL_RPATH "../${CMAKE_INSTALL_LIBDIR}")

    if (NOT BUILD_STANDALONE)
        # create alias to enable treating the library inside this project as if it were imported in namespace
        add_library(biomolecules::${sprelay_project_name}::${sprelay_project_name} ALIAS ${sprelay_project_name})
    endif()

    # install the library and export it to target list
    install(TARGETS ${sprelay_project_name} EXPORT ${sprelay_project_name}Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    if (ENABLE_COVERAGE)
        target_link_libraries(${sprelay_project_name} -fprofile-instr-generate -fcoverage-mapping)
        set_property(GLOBAL APPEND PROPERTY coverage_binaries ${sprelay_project_name})
    endif()

    # link in sanitizers
    if (ADDRESS_SANITIZE)
        target_link_libraries(${sprelay_project_name} -fsanitize=address)
    endif()
    if (THREAD_SANITIZE)
        target_link_libraries(${sprelay_project_name} -fsanitize=thread)
    endif()
    if (UB_SANITIZE)
        target_link_libraries(${sprelay_project_name} -fsanitize=undefined)
    endif()
endif()

# Documentation sources - Qt creator does not support interface libraries so the documentation sources are added to
# normal targets
if (DOXYGEN_FOUND)
    set(${sprelay_project_name}_doc_src
        ${PROJECT_SOURCE_DIR}/index.md
        ${PROJECT_SOURCE_DIR}/overview.md
        ${PROJECT_SOURCE_DIR}/style_guide.md
        ${PROJECT_SOURCE_DIR}/sprelay.dox)
    target_sources(${sprelay_project_name} PRIVATE ${${sprelay_project_name}_doc_src})
    set_source_files_properties(${${sprelay_project_name}_doc_src} PROPERTIES HEADER_FILE_ONLY TRUE)
endif()

# build core
add_subdirectory(core)

# add sources to target ${sprelay_project_name} from the gui subfolder
if (BUILD_STANDALONE OR NOT SKIP_GUI)
    include(gui/CMakeLists.txt)
endif()

if (NOT BUILD_STANDALONE)
    # export the target list into build tree
    export(EXPORT ${sprelay_project_name}Targets
        FILE "${sprelay_bin_cmake_dir}/${sprelay_project_name}Targets.cmake"
        NAMESPACE biomolecules::${sprelay_project_name}::
    )

    # install package cmake files
    install(EXPORT ${sprelay_project_name}Targets
        FILE ${sprelay_project_name}Targets.cmake
        NAMESPACE biomolecules::${sprelay_project_name}::
        DESTINATION "${sprelay_install_cmake_dir}"
        COMPONENT Develop)
endif()
