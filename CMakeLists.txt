cmake_minimum_required(VERSION 3.16)
project(NeuronSeqSampler VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(SFML REQUIRED sfml-graphics sfml-audio sfml-system sfml-window)

# Try to find TGUI, but make it optional for now
find_path(TGUI_INCLUDE_DIR TGUI/TGUI.hpp)
find_library(TGUI_LIBRARY tgui)

# Create source directory if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/src)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/assets/fonts)

# Include directories
include_directories(src)
include_directories(${SFML_INCLUDE_DIRS})

# Check if TGUI is available
if(TGUI_INCLUDE_DIR AND TGUI_LIBRARY)
    message(STATUS "Found TGUI: ${TGUI_LIBRARY}")
    include_directories(${TGUI_INCLUDE_DIR})
    set(USE_TGUI 1)
    add_definitions(-DUSE_TGUI)
else()
    message(STATUS "TGUI not found, GUI will be disabled")
    set(USE_TGUI 0)
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/Neuron.cpp
    src/Connection.cpp
    src/NeuronNetwork.cpp
    src/Visualizer.cpp
    src/AudioManager.cpp
)

set(HEADERS
    src/Neuron.h
    src/Connection.h
    src/NeuronNetwork.h
    src/Visualizer.h
    src/AudioManager.h
)

# Add GUI files only if TGUI is available
if(USE_TGUI)
    list(APPEND SOURCES src/GUI.cpp)
    list(APPEND HEADERS src/GUI.h)
endif()

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Set compiler flags for SFML
target_compile_options(${PROJECT_NAME} PRIVATE ${SFML_CFLAGS_OTHER})

# Link libraries
target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES})

if(USE_TGUI)
    target_link_libraries(${PROJECT_NAME} ${TGUI_LIBRARY})
endif()

# Copy assets to build directory
file(COPY ${CMAKE_SOURCE_DIR}/samples DESTINATION ${CMAKE_BINARY_DIR})
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets/fonts)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)