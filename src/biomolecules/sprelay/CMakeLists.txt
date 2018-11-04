project(${sprelay_project_name}_src)

# sprelay global headers #
# ---------------------- #
set(sprelay_global_target sprelay_global)
add_library(${sprelay_global_target} INTERFACE)
target_sources(${sprelay_global_target} INTERFACE
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/biomolecules/sprelay/sprelay_global.h>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/sprelay_global.h>)
target_include_directories(${sprelay_global_target} SYSTEM INTERFACE $<INSTALL_INTERFACE:include>)
target_include_directories(${sprelay_global_target} INTERFACE $<BUILD_INTERFACE:${sprelay_source_dir}>)
if (NOT BUILD_STANDALONE)
    install(TARGETS ${sprelay_global_target} EXPORT ${sprelay_global_target}Config
        # ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(EXPORT ${sprelay_global_target}Config DESTINATION share/${sprelay_global_target}/cmake)
    export(TARGETS ${sprelay_global_target} FILE ${sprelay_global_target}Config.cmake)

    install(FILES "${PROJECT_SOURCE_DIR}/sprelay_global.h"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/biomolecules/sprelay")
endif()

# sprelay gui #
# ----------- #

if (BUILD_STANDALONE OR NOT SKIP_GUI)
    # collect files
    set(${sprelay_project_name}_src "")

    # compile files connected only with standalone application only on demand
    if (BUILD_STANDALONE)
        list(APPEND ${sprelay_project_name}_src
            ${PROJECT_SOURCE_DIR}/main.cpp)
    endif()

    # create executable or dll
    if (BUILD_STANDALONE)
        add_executable(${sprelay_project_name}
            ${${sprelay_project_name}_src})
        target_compile_definitions(${sprelay_project_name} PRIVATE SPRELAY_STANDALONE)
        if (sprelay_standalone_application_link_flags)
            set_target_properties(${sprelay_project_name} PROPERTIES LINK_FLAGS
                ${sprelay_standalone_application_link_flags})
        endif()
    else()
        add_library(${sprelay_project_name} SHARED
            ${${sprelay_project_name}_src})
        target_compile_definitions(${sprelay_project_name} PRIVATE SPRELAY_MAIN_LIBRARY)
    endif()
    target_link_libraries(${sprelay_project_name}
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Threads::Threads
        sprelay_global)
    set_target_properties(${sprelay_project_name} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
    set_target_properties(${sprelay_project_name} PROPERTIES INSTALL_RPATH "../${CMAKE_INSTALL_LIBDIR}")

    install(TARGETS ${sprelay_project_name} EXPORT ${sprelay_project_name}Config
        # ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    if (NOT BUILD_STANDALONE)
        install(EXPORT ${sprelay_project_name}Config DESTINATION share/${sprelay_project_name}/cmake)
        export(TARGETS ${sprelay_project_name} FILE ${sprelay_project_name}Config.cmake)
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
