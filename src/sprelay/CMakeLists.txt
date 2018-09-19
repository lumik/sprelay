project(${sprelay_project_name}_src)

# sprelay global headers #
# ---------------------- #
set(sprelay_global_target sprelay_global)
add_library(${sprelay_global_target} INTERFACE)
target_sources(${sprelay_global_target} INTERFACE
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/sprelay/sprelay_global.h>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/sprelay_global.h>)
target_include_directories(${sprelay_global_target} SYSTEM INTERFACE $<INSTALL_INTERFACE:include>)
target_include_directories(${sprelay_global_target} INTERFACE $<BUILD_INTERFACE:${sprelay_source_dir}>)
if(NOT SPRELAY_BUILD_STANDALONE)
    install(TARGETS ${sprelay_global_target} EXPORT ${sprelay_global_target}Config
        # ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(EXPORT ${sprelay_global_target}Config DESTINATION share/${sprelay_global_target}/cmake)
    export(TARGETS ${sprelay_global_target} FILE ${sprelay_global_target}Config.cmake)

    install(FILES "${PROJECT_SOURCE_DIR}/sprelay_global.h"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/sprelay")
endif(NOT SPRELAY_BUILD_STANDALONE)

# sprelay gui #
# ----------- #

if(SPRELAY_BUILD_STANDALONE OR NOT SPRELAY_SKIP_GUI)
    # collect files
    set(${sprelay_project_name}_src "")

    # compile files connected only with standalone application only on demand
    if(SPRELAY_BUILD_STANDALONE)
        list(APPEND ${sprelay_project_name}_src
            ${PROJECT_SOURCE_DIR}/main.cpp)
    endif(SPRELAY_BUILD_STANDALONE)

    # create executable or dll
    if(SPRELAY_BUILD_STANDALONE)
        add_executable(${sprelay_project_name}
            ${${sprelay_project_name}_src})
        target_compile_definitions(${sprelay_project_name} PRIVATE SPRELAY_STANDALONE)
        if(sprelay_standalone_application_link_flags)
            set_target_properties(${sprelay_project_name} PROPERTIES LINK_FLAGS
                ${sprelay_standalone_application_link_flags})
        endif(sprelay_standalone_application_link_flags)
    else()
        add_library(${sprelay_project_name} SHARED
            ${${sprelay_project_name}_src})
        target_compile_definitions(${sprelay_project_name} PRIVATE SPRELAY_GLOBAL_LIBRARY)
    endif(SPRELAY_BUILD_STANDALONE)
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
    if(NOT SPRELAY_BUILD_STANDALONE)
        install(EXPORT ${sprelay_project_name}Config DESTINATION share/${sprelay_project_name}/cmake)
        export(TARGETS ${sprelay_project_name} FILE ${sprelay_project_name}Config.cmake)
    endif(NOT SPRELAY_BUILD_STANDALONE)
endif(SPRELAY_BUILD_STANDALONE OR NOT SPRELAY_SKIP_GUI)

# build core
add_subdirectory(core)

# add sources to target ${sprelay_project_name} from the gui subfolder
include(gui/CMakeLists.txt)
