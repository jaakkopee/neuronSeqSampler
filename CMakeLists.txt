cmake_minimum_required(VERSION 3.16)
project(NeuronSeqSampler VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



# Find required packages
find_package(SFML 3.0 REQUIRED COMPONENTS System Window Graphics Audio Network)
find_package(TGUI CONFIG REQUIRED)

# Set static libraries to match our installation
set(SFML_STATIC_LIBRARIES TRUE)
set(TGUI_STATIC_LIBRARIES TRUE)

# Create source directory if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/src)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/assets/fonts)



# Include directories
include_directories(src)




# Check if TGUI is available
if(TGUI_FOUND)
    message(STATUS "TGUI found via CMake CONFIG")
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
    src/Recorder.cpp
    src/RhythmInterpreter.cpp
    src/BeatRoot.cpp
    src/StartupAnimation.cpp
)

set(HEADERS
    src/Neuron.h
    src/Connection.h
    src/NeuronNetwork.h
    src/Visualizer.h
    src/AudioManager.h
    src/Recorder.h
    src/RhythmInterpreter.h
    src/BeatRoot.h
    src/StartupAnimation.h
)

# Add GUI files only if TGUI is available
if(USE_TGUI)
    list(APPEND SOURCES src/GUI.cpp)
    list(APPEND HEADERS src/GUI.h)
endif()

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})


# Link libraries
if(USE_TGUI)
    target_link_libraries(${PROJECT_NAME} PRIVATE TGUI::TGUI)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Graphics SFML::Audio SFML::System SFML::Window)

# Create recorder demo executable
add_executable(recorder_demo recorder_demo.cpp src/Recorder.cpp)
target_link_libraries(recorder_demo PRIVATE SFML::Audio SFML::System)

# Create debug recorder executable
add_executable(debug_recorder debug_recorder.cpp src/Recorder.cpp)
target_link_libraries(debug_recorder PRIVATE SFML::Audio SFML::System)

# Create SFML audio test executable
add_executable(test_sfml_audio test_sfml_audio.cpp)
target_link_libraries(test_sfml_audio PRIVATE SFML::Audio SFML::System)

# Copy assets to build directory
file(COPY ${CMAKE_SOURCE_DIR}/samples DESTINATION ${CMAKE_BINARY_DIR})
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets/fonts)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)