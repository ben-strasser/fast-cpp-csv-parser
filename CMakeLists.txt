cmake_minimum_required(VERSION 3.1.0)
project(fast_cpp_csv_parser)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#------------------- COMPILATION FLAGS ----------------------------------------

if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "")
    string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")
else()
    set(CMAKE_BUILD_TYPE DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O0")
endif()
message(STATUS "Building ${CMAKE_PROJECT_NAME} in ${CMAKE_BUILD_TYPE} mode")

#------------------- FILE IDENTIFICATION --------------------------------------

set(FAST_CPP_CSV_PARSER_LIB ${CMAKE_PROJECT_NAME})
set(FAST_CPP_CSV_PARSER_SRC
    csv.h
)

#------------------- LIBRARY DECLARATION --------------------------------------

add_library(${FAST_CPP_CSV_PARSER_LIB} ${FAST_CPP_CSV_PARSER_SRC})
set_target_properties(${FAST_CPP_CSV_PARSER_LIB} PROPERTIES
    LINKER_LANGUAGE CXX
)

#------------------- LIBRARY DEPENDENCY LINKING -------------------------------

option(USE_THREADS "Enable the use of threads within fast-cpp-parser" ON)

if (USE_THREADS)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)

    target_link_libraries(${FAST_CPP_CSV_PARSER_LIB}
        Threads::Threads
    )
else()
    add_definitions(-DCSV_IO_NO_THREAD)
endif()

#------------------- INCLUDE DIRECTORIES TO SEARCH PATH -----------------------

target_include_directories(${FAST_CPP_CSV_PARSER_LIB} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
