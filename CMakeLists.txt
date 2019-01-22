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
     set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()


# create macros that will be passed C/C++ code. utils.cpp and service.cpp rely on those macros
# to choose the 
OPTION(DEBUG_ON "Option to activate debug output from code" OFF)
SET(PROCESSING_DURATION CACHE STRING "Define the maximale processing time needed by the task job")
SET(FAILURES_THRESHOLD CACHE STRING "SELECT value for the FAILURES_THRESHOLD flag")
SET(RETRY_TIMEOUT CACHE STRING "Select the value to use for Retry when the Circuit breaker is open")
OPTION(TIME_UNIT_MS OFF "Select the time unit to be used in the project")
OPTION(MTHREADING OFF "Select the number of thread used by the circuit breaker" )
OPTION(USE_REMOTE_SERVICE ON "Select the number of thread used by the circuit breaker" )

set_property(CACHE PROCESSING_DURATION PROPERTY STRINGS
    30 50 100 150 200
    )
set_property(CACHE FAILURES_THRESHOLD PROPERTY STRINGS
    3 10 15
    )
set_property(CACHE RETRY_TIMEOUT PROPERTY STRINGS
    10 30 100 150
    )
set_property(CACHE MTHREADING PROPERTY STRINGS
    1 2 4 8 16
    )

if(TIME_UNIT_MS)
    set(TIME_UNIT "ms")
else()
    set(TIME_UNIT "us")
endif()
set(LOG_DIR ${PROJECT_BINARY_DIR}/log)
file(MAKE_DIRECTORY ${LOG_DIR})
if(DEBUG_ON)
    message("Adding DEBUG_ON macro definition")
    #add_definitions(-DDEBUG_ON)
    #add_definitions(-DFAILURES_THRESHOLD)
    #add_definitions(-DRETRY_TIMEOUT)
    #add_definitions(-DTIME_UNIT_MS)
    set(LOG_PREFIX "Circuitbreaker")

    configure_file(
        ${PROJECT_SOURCE_DIR}/include/config.h.in
        ${PROJECT_BINARY_DIR}/config.h
        )
    include_directories(${PROJECT_BINARY_DIR})

else()
    message("DEBUG_ON macro definition not added")
endif(DEBUG_ON)
message("Welcome to Reactor" )
message("CMAKE version in use : ${CMAKE_VERSION}")
# set the path where cmake would look for cmake module
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} include)

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
list(REMOVE_ITEM TEST_SOURCES ${PROJECT_SOURCE_DIR}/src/mainbak.cpp)

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

#add libcurl
set(CURL_LIBRARY "-lcurl")
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})
message("CULR lib : ${CURL_LIBRARIES}")


#set(BOOST_USE_STATIC_LIBS OFF)
set(BOOST_USE_MULTITHREAD ON)
#set(BOOST_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)
if(Boost_FOUND)

    include_directories(${Boost_INCLUDE_DIRS})
    #LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
endif()


add_executable (${TARGET} ${SOURCES} "src/main.cpp")
target_link_libraries(${TARGET} ${Logger_LIBRARY} ${CURL_LIBRARIES})

# add test executable 
add_executable(${TEST_TARGET} ${TEST_SOURCES} tests/main.cpp )
#target_link_libraries(${TEST_TARGET} ${TEST_LIBS})
target_link_libraries(${TEST_TARGET} ${CURL_LIBRARIES} ${Logger_LIBRARY} Boost::unit_test_framework)
target_include_directories(${TEST_TARGET} PUBLIC ${Boost_INCLUDE_DIRS})
# pass automatically macro needed by Boost to choose the link option
target_compile_definitions(${TEST_TARGET} PRIVATE "BOOST_TEST_DYN_LINK=1")


# tell CMake to generate Test command. This allow us to run ctest in the binary dir
# to execute all our registered tests.
add_test(NAME unit_test1 COMMAND ${TEST_TARGET})

