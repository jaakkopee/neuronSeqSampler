cmake_minimum_required(VERSION 3.16)
project(TestBuild)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SFML_STATIC_LIBRARIES OFF)
find_package(SFML 3.0 REQUIRED COMPONENTS System Window Graphics Audio Network)
find_package(TGUI CONFIG REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFTW3 REQUIRED fftw3)

include_directories(src)
add_definitions(-DUSE_TGUI)

add_executable(test_build 
    src/main.cpp 
    src/NeuronNetwork.cpp
    src/RhythmInterpreter.cpp
    src/BeatTracker.cpp
    src/Neuron.cpp
    src/Connection.cpp
    src/AudioManager.cpp
    src/Visualizer.cpp
    src/Recorder.cpp
    src/GUI.cpp
    src/SimpleSpectralDisplay.cpp
    src/PresetManager.cpp
    src/StartupAnimation.cpp
    src/Quantizer.cpp
    src/QuantizerWidget.cpp
)

target_link_libraries(test_build 
    SFML::System SFML::Window SFML::Graphics SFML::Audio SFML::Network
    TGUI::TGUI
    nlohmann_json::nlohmann_json
    ${FFTW3_LIBRARIES}
)

target_include_directories(test_build PRIVATE ${FFTW3_INCLUDE_DIRS})
target_compile_options(test_build PRIVATE ${FFTW3_CFLAGS_OTHER})
target_link_directories(test_build PRIVATE ${FFTW3_LIBRARY_DIRS})
if(FFTW3_LDFLAGS)
    target_link_options(test_build PRIVATE ${FFTW3_LDFLAGS})
endif()