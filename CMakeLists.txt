cmake_minimum_required(VERSION 3.8)

project("EnvironmentMapping")

include(cmake/check-compiler.cmake)

set(CMAKE_CXX_STANDARD 17)

set(
	SOURCE_FILES 
		main.cpp
		source/Light.cpp
		source/Camera.cpp
		source/Object.cpp
		source/Shader.cpp
		source/LightPosition.cpp
		source/LongitudeLatitudeMapping.cpp
		source/Renderer.cpp
)

configure_file(include/ProjectPath.h.in ${PROJECT_BINARY_DIR}/ProjectPath.h @ONLY)

include_directories("include")
if(MSVC)
   include(cmake/add-libraries-windows.cmake)
else()
   include(cmake/add-libraries-linux.cmake)
endif()

add_executable(EnvironmentMapping ${SOURCE_FILES})

if(MSVC)
   include(cmake/target-link-libraries-windows.cmake)
else()
   include(cmake/target-link-libraries-linux.cmake)
endif()

target_include_directories(EnvironmentMapping PUBLIC ${CMAKE_BINARY_DIR})