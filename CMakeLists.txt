project (Reactor CXX)
cmake_minimum_required (VERSION 2.8)
include(CTest)
include(Dart)
if(${CMAKE_VERSION} VERSION_LESS 3.8)
    message("CMake current version doesn't support C++ 17 Standard \n"
            "Setting C++17 flag manually. You may considere updating you CMake version")
    set(CMAKE_C_COMPILER gcc-7)
    set(CMAKE_CXX_COMPILER g++-7)
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")
    message("Setting C++ 17 Flags manually done !")
else()
    message("your current CMake version support C++ 17")
    set (CMAKE_CXX_STANDARD 17)
#    find_package(Threads REQUIRED)
#    if(Threads_FOUND)
#       message("Found Threads library")
#        message("Thread Include dir : " ${Threads_INCLUDE_DIR})
#        message("Thread libraries : ${Threads_LIBRARIES}" )
#    endif()
     set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()


message("Welcome to Reactor Project" )
message("CMAKE version in use : ${CMAKE_VERSION}")
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} include)
# set the path where cmake would look for cmake module
set(SOURCE_DIR ${PROJECT_SOURCE_DIR})
set(TARGET Reactor)
set(TEST_TARGET Test)
set(EXECUTABLE_OUTPUT_PATH
    ${PROJECT_BINARY_DIR}
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
set(TEST_SOURCES ${SOURCES})
list(REMOVE_ITEM TEST_SOURCES ${PROJECT_SOURCE_DIR}/src/main.cpp)


# process the CMake subdirectory
#add_subdirectory(CMake)
#include and process the commands in CMake/FindCAF.cmake
#set CAF root directory
set(USE_LOGGER ON)
set(TEST_LIBS " ")
if(USE_LOGGER)
    find_package(Logger)
    include(FindLogger)
    message("Found  FindLogger.cmake")
    message("Logger name : " ${Logger_FOUND})
    if(Logger_FOUND)
        include_directories(${Logger_INCLUDE_DIR})
        message("Logger found !!" )
        message("LOGGER_LIB" ${Logger_LIBRARY})
        set(${TEST_LIBS} ${TEST_LIBS} ${Logger_LIBRARY})
        message("LOGGER_INCLUDE_DIRS" ${Logger_INCLUDE_DIR})

    endif()
    option(USE_LOGGER "Use Logger Capabilities provided by Logger Lib" ON)
endif()


#set(BOOST_USE_STATIC_LIBS OFF)
set(BOOST_USE_MULTITHREAD ON)
#set(BOOST_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)
if(Boost_FOUND)

    message("Found Boost  on this computer")


    include_directories(${Boost_INCLUDE_DIRS})

    #message(status "** Boost Root : ${Boost_BOOSTROOT}")
    #message(status "** Boost INCLUDE_DIRECTORIES : ${Boost_INCLUDE_DIRS}")
    #message(status "** Boost library to be linked : ${Boost_LIBRARIES}")
    #message(status "** Boost unit test libs 1 : ${Boost_LIBRARY_DIRS}")
    #message(status "** Boost unit test libs 2: ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}")
    #LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
endif()


add_executable (${TARGET} ${SOURCES} "src/main.cpp")
target_link_libraries(${TARGET} ${Logger_LIBRARY})


add_executable(${TEST_TARGET} tests/main.cpp )
#target_link_libraries(${TEST_TARGET} ${TEST_LIBS})
target_link_libraries(${TEST_TARGET} ${Logger_LIBRARY} Boost::unit_test_framework)
#target_include_directories(${TEST_TARGET} PUBLIC ${Boost_INCLUDE_DIRS})
target_compile_definitions(${TEST_TARGET} PRIVATE "BOOST_TEST_DYN_LINK=1")
#target_link_libraries(${TEST_TARGET} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
add_test(NAME unit_test1 COMMAND ${TEST_TARGET})
# look for the C++ CAF library


#include (GetPrerequisites)
#get_prerequisites(${TARGET} DEPS 0 0 "" "")
#message("${TARGET} DEPENDS on : ")
#foreach(DEP ${DEPS})
#    message(${DEP})
#endforeach()
