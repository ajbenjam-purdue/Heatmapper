cmake_minimum_required(VERSION 3.15)

# Name the project and specify C++
project(Heatmapper CXX)

# Export compilation commands to json since Intellisense is being annoying
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enforce modern(er) C++ standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==========================================
# 1. OS-SPECIFIC PATHS & CONFIGURATION
# ==========================================
if(APPLE)
    # Tell CMake where wx-config is on Mac 
    set(wxWidgets_CONFIG_EXECUTABLE "/Users/ajbenj/Development/Libraries/wxWidgets-3.3.1/build-osx/wx-config")
    
    # Mac Include Paths (JSON and Eigen)
    set(CUSTOM_INCLUDE_DIRS 
        "/Users/ajbenj/Development/Libraries/"
        "/Users/ajbenj/Development/Libraries/eigen-5.0.1"
    )
elseif(WIN32)
    # Point directly to the newly installed CMake config folder
    set(wxWidgets_DIR "C:/Development/Libraries/wxWidgets-3.3.2/install_x64/lib/cmake/wxWidgets-3.3")
    
    # Windows Include Paths (JSON and Eigen)
    set(CUSTOM_INCLUDE_DIRS 
        "C:/Development/Libraries/"
        "C:/Development/Libraries/eigen-5.0.0" 
    )
endif()

# ==========================================
# 2. LOCATE WXWIDGETS
# ==========================================
if(WIN32)
    find_package(wxWidgets CONFIG REQUIRED COMPONENTS core base)
else()
    find_package(wxWidgets REQUIRED COMPONENTS core base)
endif()

if(wxWidgets_USE_FILE)
    include(${wxWidgets_USE_FILE}) # Good practice for bringing in wxWidgets macros safely
endif()

# ==========================================
# 3. ASSET PACKAGING
# ==========================================
file(GLOB_RECURSE ALL_ASSETS "assets/*") 

if(APPLE)
    # Loop through every asset and tell macOS to perfectly mirror the folder structure!
    foreach(ASSET_FILE ${ALL_ASSETS})
        file(RELATIVE_PATH REL_DIR "${CMAKE_CURRENT_SOURCE_DIR}" "${ASSET_FILE}")
        get_filename_component(DEST_DIR "${REL_DIR}" DIRECTORY)
        
        # This puts files into Resources/assets/ or Resources/assets/resistances/
        set_source_files_properties("${ASSET_FILE}" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/${DEST_DIR}")
    endforeach()
    
    # If you have a specific Mac app icon, override it to sit at the absolute root of Resources
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.icns")
        set_source_files_properties("assets/icon.icns" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    endif()

    set(APP_ASSETS ${ALL_ASSETS})
    
elseif(WIN32)
    # Automatically generate a flawless resource file with the absolute, full path!
    set(RC_FILE_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated_resource.rc")
    file(WRITE ${RC_FILE_PATH} "AppIcon ICON \"${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.ico\"\n")
    set(APP_ASSETS ${RC_FILE_PATH} ${ALL_ASSETS})

    # Tell CMake to physically copy the directories to the MSVC Release folder!
    file(COPY "assets" DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
endif()

# ==========================================
# 4. BUILD THE EXECUTABLE
# ==========================================
add_executable(Heatmapper WIN32 MACOSX_BUNDLE 
    src/App.cpp 
    src/MainFrame.cpp
    src/ThermalNode.cpp
    src/ThermalEdge.cpp
    src/ThermalNetwork.cpp
    src/ThermalSolver.cpp
    src/ThermalCanvas.cpp
    src/EdgeConfigDialog.cpp
    src/DiscretizeDialog.cpp
    ${APP_ASSETS} # <-- Pass the OS-specific asset variable here!
)

# Add the dynamic include directories we set in the if/else block above
target_include_directories(Heatmapper PRIVATE 
    ${wxWidgets_INCLUDE_DIRS}
    ${CUSTOM_INCLUDE_DIRS}
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
)

# Map target definitions and flags
target_compile_definitions(Heatmapper PRIVATE ${wxWidgets_DEFINITIONS})
target_compile_options(Heatmapper PRIVATE ${wxWidgets_CXX_FLAGS})

# Link the compiled wxWidgets libraries
target_link_libraries(Heatmapper PRIVATE ${wxWidgets_LIBRARIES})

# ==========================================
# 5. OS-SPECIFIC PROPERTIES
# ==========================================
if(APPLE)
    # Use custom Info.plist for Mac
    set_target_properties(Heatmapper PROPERTIES 
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
    )
endif()