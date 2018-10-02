##################
##  enum_flags  ##
##################

if(SPRELAY_INSTALL_ENUM_FLAGS)
    message(STATUS "Subproject: enum_flags...")
    # setup enum_flags at configure time
    # The following commands configures external project without ExternalProject_Add
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/enum_flagsBuild")
    execute_process(COMMAND ${CMAKE_COMMAND} ${CMAKE_CURRENT_LIST_DIR}/enum_flags
            -G "${CMAKE_GENERATOR}"
            -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
            -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH=${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/enum_flagsBuild"
        RESULT_VARIABLE result)
    if(result)
        message(FATAL_ERROR "CMake step for enum_flags failed: ${result}")
    endif()

    # The following commands uses ExternalProject_add through at configure time invoked cmake project.
#    configure_file(
#        ${CMAKE_CURRENT_LIST_DIR}/enum_flags.CMakeLists.txt.in
#        ${CMAKE_BINARY_DIR}/enum_flagsCreate/CMakeLists.txt)
#    execute_process(COMMAND ${CMAKE_COMMAND} .
#            -G "${CMAKE_GENERATOR}"
#            -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
#            -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
#            -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
#            -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH=${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
#        RESULT_VARIABLE result
#        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/enum_flagsCreate)
#    if(result)
#        message(FATAL_ERROR "CMake step for enum_flags failed: ${result}")
#    endif()
#    execute_process(COMMAND ${CMAKE_COMMAND} --build .
#        RESULT_VARIABLE result
#        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/enum_flagsCreate)
#    if(result)
#        message(FATAL_ERROR "Build step for enum_flags failed: ${result}")
#    endif()

    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} --build . --target install
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/enum_flagsBuild)")
endif(SPRELAY_INSTALL_ENUM_FLAGS)


message(STATUS "Subproject: enum_flags... DONE")

find_package(enum_flags 3
    COMPONENTS Develop
    HINTS ${CMAKE_BINARY_DIR}/enum_flagsBuild
    REQUIRED)
