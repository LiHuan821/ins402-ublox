cmake_minimum_required(VERSION 3.5.0)
set(CMAKE_BUILD_TYPE Release)

project (TeseoProgrammer)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(UNIX)
    add_definitions(-Wformat-security)
endif()

include_directories(
    ./src
)

file(GLOB SOURCES
            "./src/*.h"
            "./src/*.c"
            "./src/fwupg/*.c"
            "./src/fwupg/*.h"
            "./src/teseo2/*.c"
            "./src/teseo2/*.h"
            "./src/teseo3/*.c"
            "./src/teseo3/*.h"
            "./src/teseo5/*.c"
            "./src/teseo5/*.h"
    )
    
if(WIN32)
            file(GLOB SRC_BSP "./src/bsp/tp_uart_win.c" )
endif()

if(UNIX)
            file(GLOB SRC_BSP "./src/bsp/tp_uart_linux.c" )
endif()
    
add_executable(TeseoProgrammer ${SOURCES} ${SRC_BSP})


