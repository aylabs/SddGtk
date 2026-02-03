cmake_minimum_required(VERSION 3.16)
project(hello-app VERSION 1.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)
pkg_check_modules(CHECK check QUIET)

# Include directories
include_directories(src ${CMAKE_BINARY_DIR})

# Generate config.h
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_BINARY_DIR}/config.h"
)

# GResource compilation (requires glib-compile-resources)
find_program(GLIB_COMPILE_RESOURCES glib-compile-resources REQUIRED)

add_custom_command(
    OUTPUT hello-resources.c
    COMMAND ${GLIB_COMPILE_RESOURCES}
        --target=hello-resources.c
        --generate-source
        --c-name=hello_resources
        ${CMAKE_CURRENT_SOURCE_DIR}/src/hello-app/hello-app.gresource.xml
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/hello-app
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/hello-app/hello-app.gresource.xml
)

# GTK utilities library
add_library(gtk-utils STATIC
    src/lib/gtk-utils.c
)
target_link_libraries(gtk-utils ${GTK4_LIBRARIES})
target_include_directories(gtk-utils PRIVATE ${GTK4_INCLUDE_DIRS})

# Image processing library for B&W conversion
add_library(image-processing STATIC
    src/lib/image-processing.c
)
target_link_libraries(image-processing ${GTK4_LIBRARIES})
target_include_directories(image-processing PRIVATE ${GTK4_INCLUDE_DIRS})

# Main executable
add_executable(hello-app
    src/hello-app/main.c
    src/hello-app/hello-application.c
    src/hello-app/hello-window.c
    src/hello-app/hello-image-viewer.c
    ${CMAKE_BINARY_DIR}/hello-resources.c
)

target_link_libraries(hello-app 
    ${GTK4_LIBRARIES} 
    gtk-utils
    image-processing
)
target_include_directories(hello-app PRIVATE ${GTK4_INCLUDE_DIRS})
target_compile_options(hello-app PRIVATE ${GTK4_CFLAGS_OTHER})

# Install target
install(TARGETS hello-app DESTINATION bin)

# Tests (if Check framework is available)
if(CHECK_FOUND)
    enable_testing()
    
    add_executable(test-hello-application
        tests/unit/test-hello-application.c
    )
    target_link_libraries(test-hello-application 
        ${GTK4_LIBRARIES} 
        ${CHECK_LIBRARIES}
        gtk-utils
    )
    target_include_directories(test-hello-application PRIVATE ${GTK4_INCLUDE_DIRS} ${CHECK_INCLUDE_DIRS})
    
    add_executable(test-hello-window
        tests/unit/test-hello-window.c
    )
    target_link_libraries(test-hello-window 
        ${GTK4_LIBRARIES} 
        ${CHECK_LIBRARIES}
        gtk-utils
    )
    target_include_directories(test-hello-window PRIVATE ${GTK4_INCLUDE_DIRS} ${CHECK_INCLUDE_DIRS})
    
    add_executable(test-image-processing
        tests/unit/test-image-processing.c
    )
    target_link_libraries(test-image-processing 
        ${GTK4_LIBRARIES} 
        ${CHECK_LIBRARIES}
        image-processing
    )
    target_include_directories(test-image-processing PRIVATE ${GTK4_INCLUDE_DIRS} ${CHECK_INCLUDE_DIRS})
    
    add_executable(test-image-viewer-bw
        tests/unit/test-image-viewer-bw.c
    )
    target_link_libraries(test-image-viewer-bw 
        ${GTK4_LIBRARIES} 
        ${CHECK_LIBRARIES}
        gtk-utils
        image-processing
    )
    target_include_directories(test-image-viewer-bw PRIVATE ${GTK4_INCLUDE_DIRS} ${CHECK_INCLUDE_DIRS})
    
    add_test(NAME test-hello-application COMMAND test-hello-application)
    add_test(NAME test-hello-window COMMAND test-hello-window)
    add_test(NAME test-image-processing COMMAND test-image-processing)
    add_test(NAME test-image-viewer-bw COMMAND test-image-viewer-bw)
endif()