cmake_minimum_required(VERSION 3.5)

project(raspicaminterface LANGUAGES CXX)

find_package(raspicam REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(raspicaminterface SHARED
  raspicaminterface_global.h
  raspicaminterface.cpp
  raspicaminterface.h
)
target_link_libraries(raspicaminterface raspicam)
target_compile_definitions(raspicaminterface PRIVATE RASPICAMINTERFACE_LIBRARY)

add_executable( testinterface testinterface.cpp)
target_link_libraries( testinterface raspicaminterface )

    INSTALL(TARGETS raspicaminterface
	RUNTIME DESTINATION bin COMPONENT main
	LIBRARY DESTINATION lib PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE COMPONENT main
	ARCHIVE DESTINATION lib COMPONENT main)

#Installation of all header files	
INSTALL(FILES raspicaminterface.h raspicaminterface_global.h
        DESTINATION include/raspicam
	COMPONENT main)

