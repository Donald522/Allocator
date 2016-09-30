cmake_minimum_required (VERSION 2.8)

project (allocator)

#DEBUG by default
if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_DEBUG_POSTFIX "_d")

set(EXECUTABLE_NAME "allocator")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O1")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_LINUX")
ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include_directories(${PROJECT_SOURCE_DIR}/include/)

#set(SRC_LIST src/app/Application.cpp)

#file(GLOB SOURCES src/*.cpp)
SET(SOURCES
        src/allocator.cpp
        src/main.cpp
    )



set(ALLOCATOR_LIB_NAME "alloc")
add_library(${ALLOCATOR_LIB_NAME} ${SOURCES} ${PROJECT_SOURCE_DIR}/include/)
add_executable(${EXECUTABLE_NAME} "src/main.cpp")
target_link_libraries(${EXECUTABLE_NAME}  ${ALLOCATOR_LIB_NAME})

# tests
set(TESTS
        allocator_test
        )

# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})


foreach(TEST ${TESTS})
    add_executable(${TEST} tests/${TEST}.cpp )
    #set_property(TARGET ${TEST} PROPERTY DEBUG_POSTFIX _d)
    target_link_libraries(
            ${TEST}
            ${GTEST_LIBRARIES}
            ${ALLOCATOR_LIB_NAME}
            pthread
    )
    install(TARGETS ${TEST} DESTINATION bin)
endforeach(TEST)


# Install target
install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin)