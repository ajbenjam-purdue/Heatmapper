cmake_minimum_required(VERSION 3.15)

# Name your project and specify C++
project(Heatmapper CXX)

# Enforce modern C++ standards
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Locate wxWidgets
find_package(wxWidgets REQUIRED COMPONENTS core base)

# Create the executable
add_executable(Heatmapper WIN32 MACOSX_BUNDLE src/main.cpp)

# --- THE MAPPED DEPENDENCIES ---
# 1. Map the headers
target_include_directories(Heatmapper PRIVATE ${wxWidgets_INCLUDE_DIRS})
# 2. Map the target definitions (THIS FIXES THE 'NO TARGET' ERROR)
target_compile_definitions(Heatmapper PRIVATE ${wxWidgets_DEFINITIONS})
# 3. Map any extra compiler flags
target_compile_options(Heatmapper PRIVATE ${wxWidgets_CXX_FLAGS})
# -------------------------------

# Link the wxWidgets libraries to your app
target_link_libraries(Heatmapper PRIVATE ${wxWidgets_LIBRARIES})

# Tell CMake to use your custom Info.plist
set_target_properties(Heatmapper PROPERTIES 
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
)