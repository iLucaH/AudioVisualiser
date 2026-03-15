# Run with cmake -B build -G "Visual Studio 17 2022"

cmake_minimum_required(VERSION 3.24)

# Change to your project name
project(AudioVisualiser VERSION 0.0.1)


set(CMAKE_CXX_STANDARD 20)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# We're going to use CPM as our package manager to bring in JUCE
# Check to see if we have CPM installed already.  Bring it in if we don't.
set(CPM_DOWNLOAD_VERSION 0.34.0)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if (NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
endif ()

include(${CPM_DOWNLOAD_LOCATION})

# Bring in JUCE locally
CPMAddPackage(
    NAME juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG origin/master
)

# Install Microsoft.Web.WebView2 NuGet package to allow WebViews on Windows
if (MSVC)
    find_program(NUGET_EXE NAMES nuget)
    if(NOT NUGET_EXE)
        message(FATAL_ERROR "NUGET.EXE not found. Please install NuGet.")
    endif()
	message(STATUS "Setting up WebView dependencies")
	execute_process(COMMAND powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/DownloadWebView2.ps1
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	RESULT_VARIABLE DOWNLOAD_WEBVIEW2_RESULT)

	if (NOT DOWNLOAD_WEBVIEW2_RESULT EQUAL 0)
		message(FATAL_ERROR "Failed to download Microsoft.Web.WebView2 NuGet package. Result: ${DOWNLOAD_WEBVIEW2_RESULT}")
	endif()
endif()

# Get ffmpeg static lib
include(FetchContent)

set(FFMPEG_URL 
	https://github.com/ShiftMediaProject/FFmpeg/releases/download/7.1/libffmpeg_7.1_msvc17_x64.zip
)

FetchContent_Declare(
    ffmpeg
    URL ${FFMPEG_URL}
)

FetchContent_MakeAvailable(ffmpeg)

set(FFMPEG_INCLUDE_DIR "${ffmpeg_SOURCE_DIR}/include")
set(FFMPEG_LIB_DIR "${ffmpeg_SOURCE_DIR}/lib/x64")

# winget install -e --id Nvidia.CUDA
# choco install cuda -y
find_package(CUDAToolkit REQUIRED)

# Make sure you include any new source files here
set(SourceFiles
	Source/AskAI.h
	Source/AVAPIResolver.h
	Source/AVIOHandler.h
	Source/Classic1_2D.h
	Source/Classic2_2D.h
	Source/Classic3_2D.h
	Source/Classic4_2D.h
	Source/CreateVideoComponent.h
	Source/GlobalSocketHandler.h
	Source/LoginComponent.h
	Source/Mesh.h
	Source/OpenGLComponent.cpp
	Source/OpenGLComponent.h
	Source/PluginEditor.cpp
	Source/PluginEditor.h
	Source/PluginProcessor.cpp
	Source/PluginProcessor.h
	Source/RenderHeaders.h
	Source/RenderObject3D.h
	Source/RenderProfileComponent.cpp
	Source/RenderProfileComponent.h
	Source/RenderState.cpp
	Source/RenderState.h
	Source/RenderState2D.cpp
	Source/RenderState2D.h
	Source/RenderState3D.cpp
	Source/RenderState3D.h
	Source/RingBuffer.h
	Source/SDF_1_2D.h
	Source/SelectorTabPanel.cpp
	Source/SelectorTabPanel.h
	Source/Settings.h
	Source/SettingsComponent.h
	Source/StrHelper.h
	Source/Texture.h
	Source/TimeDomain1_2D.h
	Source/TimeDomain2_2D.h
	Source/TimeDomain3_2D.h
	Source/tv.png
	Source/TVImageOverlay.h
	Source/ui.zip
	Source/VideoEncoder.cpp
	Source/VideoEncoder.h
	Source/WebViewHelper.h
)

set(WEBVIEW_FILES_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ui/public")

set(WEBVIEW_FILES_ZIP_NAME "ui.zip")
set(TARGET_WEBVIEW_FILES_ZIP_PATH "${CMAKE_BINARY_DIR}/${WEBVIEW_FILES_ZIP_NAME}")

cmake_path(ABSOLUTE_PATH WEBVIEW_FILES_SOURCE_DIR NORMALIZE OUTPUT_VARIABLE PUBLIC_PATH)

add_custom_target(ZipWebViewFiles
    COMMAND
        ${CMAKE_COMMAND} -E tar cvf
        "${TARGET_WEBVIEW_FILES_ZIP_PATH}"
        --format=zip
        "."
    BYPRODUCTS
        "${TARGET_WEBVIEW_FILES_ZIP_PATH}"
    WORKING_DIRECTORY
        "${PUBLIC_PATH}"
    COMMENT "Zipping WebView files..."
    VERBATIM
)

# Change these to your own preferences
juce_add_plugin(${PROJECT_NAME}
        COMPANY_NAME iLucaH
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        JUCE_VST3_CAN_REPLACE_VST2 FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE
        PLUGIN_MANUFACTURER_CODE Tap1
        PLUGIN_CODE Reg0
        FORMATS VST3 AU Standalone
        PRODUCT_NAME "AudioVisualiser"   
		NEEDS_WEB_BROWSER TRUE
		# NEEDS_WEBVIEW2 TRUE		
)

# How we want our SourceFiles to appear in our IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SourceFiles})

# Make the SourceFiles buildable
target_sources(${PROJECT_NAME} 
	PRIVATE 
		${SourceFiles}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
		"${CMAKE_CURRENT_SOURCE_DIR}/build/${PROJECT_NAME}_artefacts/JuceLibraryCode"
		${CMAKE_CURRENT_SOURCE_DIR}/build/juce_binarydata_WebViewFiles/JuceLibraryCode
		${CUDA_INCLUDE_DIRS}
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/packages/Microsoft.Web.WebView2.1.0.1901.177/build/native/include"
		${FFMPEG_INCLUDE_DIR}
)

# Package web UI sources as binary data
juce_add_binary_data(WebViewFiles
    HEADER_NAME WebViewFiles.h
    NAMESPACE webview_files
    SOURCES 
		${TARGET_WEBVIEW_FILES_ZIP_PATH}
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/tv.png"
)
add_dependencies(WebViewFiles ZipWebViewFiles)

# These are some toggleable options from the JUCE CMake API
target_compile_definitions(${PROJECT_NAME}
    PUBLIC
		JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        # This will enable WebView2 as the WebView backend on Windows
        JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1
		DONT_SET_USING_JUCE_NAMESPACE=1
)

target_link_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
		${FFMPEG_LIB_DIR}
)

# JUCE libraries to bring into our project
target_link_libraries(${PROJECT_NAME}
    PUBLIC
		juce::juce_audio_utils
		juce::juce_dsp
		juce::juce_opengl
		juce::juce_audio_processors
	PRIVATE
        WebViewFiles
		${FFMPEG_LIB_DIR}/libswresample.lib
		${FFMPEG_LIB_DIR}/libavcodec.lib
		${FFMPEG_LIB_DIR}/libavformat.lib
		${FFMPEG_LIB_DIR}/libavutil.lib
		${FFMPEG_LIB_DIR}/libswscale.lib
		CUDA::cuda_driver
		"${CMAKE_CURRENT_SOURCE_DIR}/packages/Microsoft.Web.WebView2.1.0.1901.177/build/native/x64/WebView2LoaderStatic.lib"
)

juce_generate_juce_header(${PROJECT_NAME})

if (MSVC)
    # Generate PDB files in Release mode
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT AudioVisualiser_Standalone)
