project (Reactor CXX)
cmake_minimum_required (VERSION 2.8)
set (CMAKE_CXX_STANDARD 14)
message("Welcome to Reactor Project" )
message("CMAKE version in use : ${CMAKE_VERSION}")
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} include)
set(SOURCE_DIR ${PROJECT_SOURCE_DIR})
set(TARGET Reactor)
set(EXECUTABLE_OUTPUT_PATH
    ${PROJECT_BINARY_DIR}/bin
    CACHE PATH
    "Single directory for all libraries")
set(LIBRARY_OUTPUT_DIRECTORY
    ${PROJECT_BINARY_DIR}/lib
    CACHE PATH
    "Single directory for all libraries")
include_directories(include)
file(GLOB_RECURSE REACTOR_HEADERS "${SOURCE_DIR}/include/*.h")
file (GLOB REACTOR_SOURCES "${SOURCE_DIR}/*.cpp" "${SOURCE_DIR}/include/*.h")
#set(SOURCES ${REACTOR_SOURCES})
file( GLOB SOURCES "src/*.cpp" "include/*.h")
message("current header files : ${REACTOR_HEADERS}")
message("current source files : ${REACTOR_SOURCES}")
message("current project source directory : ${PROJECT_SOURCE_DIR}")
message("current source directory : ${SOURCE_DIR}")
message("current source  : ${SOURCES}")

# process the CMake subdirectory
#add_subdirectory(CMake)
#include and process the commands in CMake/FindCAF.cmake
#set CAF root directory
set( CAF_ROOT_DIR $ENV{CAF_LIB})
if(EXISTS "${CMAKE_MODULE_PATH}/FindCAF.cmake")
    message (STATUS "Found FindCAF.cmake in CMake directory")
    find_package(CAF COMPONENTS core io openssl REQUIRED)
    include(FindCAF)
    message("CAF library PATH : ${CAF_LIBRARIES}")
else()
    message (FATAL_ERROR "FindCAF.cmake not found")
endif()
if(${CAF_FOUND})
    set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CAF_INCLUDE_DIRS})
    include_directories(${CAF_INCLUDE_DIRS})
    message("CAF include directory : ${CAF_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "CAF not found")
endif()

add_executable (${TARGET} ${SOURCES} "src/main.cpp")
target_link_libraries(${TARGET} ${CAF_LIBRARIES})

# look for the C++ CAF library
