cmake_minimum_required(VERSION 3.15)
project(Heatmapper VERSION 0.2.1 LANGUAGES CXX)

# Export compilation commands to json for IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enforce modern(er) C++ standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find Packages
find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE}) 
find_package(Eigen3 REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(BZip2 REQUIRED)

# Pack assets
file(GLOB_RECURSE ALL_ASSETS "assets/*") 

if(APPLE)
    foreach(ASSET_FILE ${ALL_ASSETS})
        file(RELATIVE_PATH REL_DIR "${CMAKE_CURRENT_SOURCE_DIR}" "${ASSET_FILE}")
        get_filename_component(DEST_DIR "${REL_DIR}" DIRECTORY)
        set_source_files_properties("${ASSET_FILE}" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/${DEST_DIR}")
    endforeach()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.icns")
        set_source_files_properties("assets/icon.icns" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    endif()

    set(APP_ASSETS ${ALL_ASSETS})

elseif(WIN32)
    set(RC_FILE_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated_resource.rc")
    file(WRITE ${RC_FILE_PATH} "AppIcon ICON \"${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.ico\"\n")
    set(APP_ASSETS ${RC_FILE_PATH} ${ALL_ASSETS})

    file(COPY "assets" DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
endif()

# Build executable
file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "src/*.h")

if(APPLE)
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${APP_ASSETS})
elseif(WIN32)
    # Hide terminal on windows
    add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} ${APP_ASSETS}) 
else()
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${APP_ASSETS})
endif()

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${wxWidgets_LIBRARIES}
    Eigen3::Eigen
    nlohmann_json::nlohmann_json
    BZip2::BZip2
)

# OS Properties
if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES 
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
    )
endif()