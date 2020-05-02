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

if (DEFINED CMAKE_SYSROOT)
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wall -Wl,-rpath-link,${CMAKE_SYSROOT}/opt/vc/lib")
    find_package(raspicaminterface REQUIRED)
else()
    add_library(raspicaminterface raspicaminterfacedummy.cpp raspicaminterfacedummy.h)
endif()

find_package(Qt5 COMPONENTS Core Gui Network REQUIRED)

add_executable(pientertain main.cpp
    hueauthentication.cpp
    hueauthentication.hh
    lightgroup.cpp
    lightgroup.hh
    huestream.cpp
    huestream.hh
    lightpacket.cpp
    lightpacket.hh
    videosource.cpp
    videosource.hh
    videosourceimage.cpp
    videosourceimage.hh
    videosourceraspberrycam.cpp
    videosourceraspberrycam.hh
    restserver.cpp
    restserver.hh
    pientertain.cpp
    pientertain.hh
)

target_link_libraries(pientertain raspicaminterface Qt5::Network Qt5::Gui Qt5::Core)

INSTALL(TARGETS pientertain
    RUNTIME DESTINATION bin COMPONENT main
    LIBRARY DESTINATION lib PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE COMPONENT main
    ARCHIVE DESTINATION lib COMPONENT main)
