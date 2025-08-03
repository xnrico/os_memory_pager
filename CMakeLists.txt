cmake_minimum_required(VERSION 3.15)
project(memory_pager)

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(TEST_DIR ${PROJECT_SOURCE_DIR}/test)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories(${INCLUDE_DIR})

aux_source_directory(${SRC_DIR} SRC_FILES)
aux_source_directory(${TEST_DIR} TEST_FILES)

add_library(libvm_app STATIC IMPORTED)
add_library(libvm_pager STATIC IMPORTED)

if(APPLE)
    add_compile_options(-D_XOPEN_SOURCE -Wall -fno-builtin)
    set_target_properties(libvm_app PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/lib/libvm_app_macos.o)
    set_target_properties(libvm_pager PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/lib/libvm_pager_macos.o)
else()
    set_target_properties(libvm_app PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/lib/libvm_app.o)
    set_target_properties(libvm_pager PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/lib/libvm_pager.o)
endif()

add_executable(pager ${SRC_FILES})
target_link_libraries(pager libvm_pager ${CMAKE_DL_LIBS})

# Add a build target for each test program
# Find all test cpp files
file(GLOB TEST_FILES "${PROJECT_SOURCE_DIR}/test/*.cpp")

foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME)
    string(REGEX REPLACE "\\.cpp$" "" TEST_NAME ${TEST_NAME})
    add_executable(${TEST_NAME} ${TEST_FILE})
    target_link_libraries(${TEST_NAME} libvm_app ${CMAKE_DL_LIBS})
endforeach()

