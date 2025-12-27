cmake_minimum_required(VERSION 3.15)
    project(c_compiler LANGUAGES C)
    
    # Set C standard
    set(CMAKE_C_STANDARD 99)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    
    # Find vcpkg packages if available
    find_package(PkgConfig QUIET)
    
    # Include directories
    include_directories(src)
    
    file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.c")

    # Add executable
    add_executable(c_compiler 
        ${SOURCES}
    )
    
    # Compiler-specific options
    if(MSVC)
        target_compile_options(c_compiler PRIVATE /W4)
    else()
        target_compile_options(c_compiler PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    
    # Debug configuration
    set(CMAKE_BUILD_TYPE Debug)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(c_compiler PRIVATE DEBUG)
        if(NOT MSVC)
            target_compile_options(c_compiler PRIVATE -g -O0)
        endif()
    endif()
    
    # Enable testing
    enable_testing()
    
    # Install target
    #install(TARGETS c_compiler  RUNTIME DESTINATION bin)
        