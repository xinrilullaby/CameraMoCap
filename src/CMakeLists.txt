cmake_minimum_required(VERSION 3.10.0)

project(CameraMoCap VERSION 0.0.1)

add_executable(main WIN32 
    main.cpp
    user_alerts.c
    util.c
    log.c
)

if (MSVC)
    add_compile_options(/SUBSYSTEM:CONSOLE)
endif()

target_link_libraries(main ${OPENGL32_LIB_PATH} ${GDI32_LIB_PATH} ../lib/glew32s ../lib/glfw3 "../lib/freeglutd")

add_library(freeglutd SHARED IMPORTED)

