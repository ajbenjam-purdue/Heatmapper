cmake_minimum_required(VERSION 3.15)

# Name the project and specify C++
project(Heatmapper CXX)

# Export compilation commands to json since Intellisense is being annoying
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enforce modern(er) C++ standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Locate wxWidgets
find_package(wxWidgets REQUIRED COMPONENTS core base)

# Define Mac icon (haven't made one yet)
# set(APP_ICON "assets/AppIcon.icns")
# set_source_files_properties(${APP_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

# Create the executable w/ all source files
add_executable(Heatmapper WIN32 MACOSX_BUNDLE 
    src/App.cpp 
    src/MainFrame.cpp
    src/ThermalNode.cpp
    src/ThermalEdge.cpp
    src/ThermalNetwork.cpp
    src/ThermalSolver.cpp
    src/ThermalCanvas.cpp
)

target_include_directories(Heatmapper PRIVATE 
    ${wxWidgets_INCLUDE_DIRS}
    /Users/ajbenj/Development/Libraries/
    /Users/ajbenj/Development/Libraries/eigen-5.0.1
)

# Map target definitions and flags
target_compile_definitions(Heatmapper PRIVATE ${wxWidgets_DEFINITIONS})
target_compile_options(Heatmapper PRIVATE ${wxWidgets_CXX_FLAGS})

# Link the compiled wxWidgets libraries
target_link_libraries(Heatmapper PRIVATE ${wxWidgets_LIBRARIES})

# Use custom Info.plist
set_target_properties(Heatmapper PROPERTIES 
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
)