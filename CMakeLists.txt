project(pientertain)
cmake_minimum_required(VERSION 3.15)

SET(pientertain_VERSION_MAJOR 0)
SET(pientertain_VERSION_MINOR 0)
SET(pientertain_VERSION_PATCH 1)

SET(EXECUTABLE_OUTPUT_PATH "${pientertain_BINARY_DIR}/bin/" CACHE PATH "Single output directory for all executables")
SET(LIBRARY_OUTPUT_PATH "${pientertain_BINARY_DIR}/lib" CACHE PATH "where to put libraries")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${pientertain_BINARY_DIR}/lib CACHE PATH "where to put libraries" FORCE)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${pientertain_BINARY_DIR}/bin CACHE PATH "where to put binaries" FORCE)

set(CMAKE_AUTOMOC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Network REQUIRED)

add_executable(pientertain main.cpp
    hueauthentication.cpp
    hueauthentication.hh
    lightgroup.cpp
    lightgroup.hh
    huestream.cpp
    huestream.hh
    lightpacket.cpp
    lightpacket.hh
)

target_link_libraries(pientertain Qt5::Network Qt5::Gui Qt5::Core )
