cmake_minimum_required(VERSION 3.23)
project(TauCOM VERSION 0.2.3 LANGUAGES CXX C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Option for building shared or static library
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(USE_TAU_UTILS "Use TauUtils as a dependency" OFF)

# We use this to check for some compiler flags, mostly to disable warnings.
include(CheckCCompilerFlag)
# This is a helper utility for generating the folder layout in VS.
include(GenVsFilters)
include(SetCompileFlags)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c" "src/*.rc")
file(GLOB_RECURSE HEADERS "include/*.hpp" "include/*.h" "include/*.inl")

# Setup the Dynamic and Static libraries with the source files.
add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Generate the "filters" for VS. This basically just creates an intuitive folder layout.
GenVsFilters(SOURCES)
GenVsFilters(HEADERS)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
target_sources(${PROJECT_NAME} PUBLIC FILE_SET "HEADERS" BASE_DIRS "include" FILES ${HEADERS})

# Set the include directory.
target_include_directories(${PROJECT_NAME} PUBLIC include)
# Set the source directory.
target_include_directories(${PROJECT_NAME} PRIVATE src)

if(USE_TAU_UTILS)
    find_package(TauUtils REQUIRED)

    target_link_libraries(${PROJECT_NAME} PUBLIC tauutils::tauutils)
endif()

SetCompileFlags(${PROJECT_NAME} PUBLIC PRIVATE ${BUILD_SHARED_LIBS})

if(BUILD_SHARED_LIBS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DTAU_COM_BUILD_SHARED)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DTAU_COM_BUILD_STATIC)
endif()

if(USE_TAU_UTILS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DTAU_COM_USE_TAU_UTILS)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

install(TARGETS ${PROJECT_NAME} 
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    FILE_SET HEADERS
)

add_library(TauCOM::TauCOM ALIAS ${PROJECT_NAME})
