cmake_minimum_required(VERSION 3.22)
project(SPVinylLab VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
if(NOT WIN32 OR NOT MSVC OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "SP Vinyl Lab requires Windows MSVC x64.")
endif()
include(FetchContent)
set(JUCE_SOURCE_DIR "" CACHE PATH "Optional local JUCE 8.0.6 source directory")
if(JUCE_SOURCE_DIR)
    add_subdirectory("${JUCE_SOURCE_DIR}" JUCE)
else()
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.6 GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()
juce_add_plugin(SPVinylLab
    COMPANY_NAME "SP Vinyl Lab"
    BUNDLE_ID "com.spvinyllab.effect"
    PLUGIN_MANUFACTURER_CODE Spvl
    PLUGIN_CODE Spv1
    PRODUCT_NAME "SP Vinyl Lab"
    IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    FORMATS VST3 VST3_CATEGORIES Fx)
file(GLOB DSP_SOURCES CONFIGURE_DEPENDS Source/DSP/*.cpp)
file(GLOB GUI_SOURCES CONFIGURE_DEPENDS Source/GUI/*.cpp)
target_sources(SPVinylLab PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp ${DSP_SOURCES} ${GUI_SOURCES})
target_include_directories(SPVinylLab PUBLIC Source)
target_compile_definitions(SPVinylLab PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(SPVinylLab PRIVATE juce::juce_audio_utils PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# Export evaluated JUCE target paths, so CI never guesses a build artefact path.
if(NOT TARGET SPVinylLab_VST3)
    message(FATAL_ERROR "Expected VST3 target SPVinylLab_VST3 was not created.")
endif()
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/ci/$<CONFIG>/vst3-paths.json"
    CONTENT "{\n  \"target\": \"SPVinylLab_VST3\",\n  \"configuration\": \"$<CONFIG>\",\n  \"binary\": \"$<TARGET_FILE:SPVinylLab_VST3>\",\n  \"bundle\": \"$<GENEX_EVAL:$<TARGET_PROPERTY:SPVinylLab_VST3,JUCE_PLUGIN_ARTEFACT_FILE>>\"\n}\n")

option(SPV_BUILD_TESTS "Build processor regression tests" ON)
if(SPV_BUILD_TESTS)
    enable_testing()
    juce_add_console_app(SPVinylLabTests PRODUCT_NAME "SP Vinyl Lab Tests")
    target_sources(SPVinylLabTests PRIVATE Tests/ProcessorTests.cpp)
    target_link_libraries(SPVinylLabTests PRIVATE SPVinylLab juce::juce_audio_utils)
    add_test(NAME ProcessorRegression COMMAND SPVinylLabTests)
    juce_add_console_app(SPVinylLabGuiCapture PRODUCT_NAME "SP Vinyl Lab GUI Capture")
    target_sources(SPVinylLabGuiCapture PRIVATE Tests/GuiCapture.cpp)
    target_link_libraries(SPVinylLabGuiCapture PRIVATE SPVinylLab juce::juce_audio_utils)
    add_test(NAME GuiBindingsAndCapture COMMAND SPVinylLabGuiCapture "${CMAKE_BINARY_DIR}/GuiCaptures")
endif()
