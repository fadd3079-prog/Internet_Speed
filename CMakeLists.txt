cmake_minimum_required(VERSION 3.20)

project(InternetSpeed
    VERSION 1.0.0
    LANGUAGES CXX
)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Windows Unicode
add_definitions(
    -DUNICODE
    -D_UNICODE
)

# Source files
add_executable(InternetSpeed WIN32
    src/main.cpp
    src/Network.cpp
    src/Network.h
    src/Taskbar.cpp
    src/Taskbar.h
    src/MeterWindow.cpp
    src/MeterWindow.h
    resources/app.rc
)

# Windows system libraries
target_link_libraries(InternetSpeed PRIVATE
    user32
    gdi32
    shell32
    iphlpapi
)

# Compiler-specific optimizations
if(MSVC)
    target_compile_options(InternetSpeed PRIVATE
        /W4
        /O2
        /GL
        /Gy
    )

    target_link_options(InternetSpeed PRIVATE
        /LTCG
        /OPT:REF
        /OPT:ICF
        /SUBSYSTEM:WINDOWS
        /DYNAMICBASE
        /NXCOMPAT
    )

elseif(MINGW)
    target_compile_options(InternetSpeed PRIVATE
        -O2
        -ffunction-sections
        -fdata-sections
    )

    target_link_options(InternetSpeed PRIVATE
        -Wl,--gc-sections
        -mwindows
    )
endif()

# Put final executable in a simple bin directory
set_target_properties(InternetSpeed PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Keep intermediate files organized
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")