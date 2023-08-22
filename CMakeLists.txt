cmake_minimum_required(VERSION 3.15)
project(TauCOM VERSION 0.1.0 LANGUAGES CXX C)

include(CheckCCompilerFlag)

add_subdirectory(libs/TauUtils)

file(GLOB SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp" "include/*.h" "include/*.inl")
file(GLOB HEADERS_BASE "include/*.hpp" "include/*.inl")

set(TAU_COM_SOURCE_FILES ${SOURCES} ${HEADERS_BASE})

add_library(${PROJECT_NAME} ${TAU_COM_SOURCE_FILES})

foreach(_source IN ITEMS ${HEADERS})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("include\\${_group_path}" FILES "${_source}")
endforeach()

source_group("src" FILES ${SOURCES})
# set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER ${HEADERS_BASE})

target_link_libraries(${PROJECT_NAME} TauUtilsDynamicShared)

# Set the include directory.
target_include_directories(${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
# Set the source directory.
target_include_directories(${PROJECT_NAME} PRIVATE src)

# Set C++20
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
# Enable PIC
#target_compile_features(${PROJECT_NAME} PUBLIC POSITION_INDEPENDENT_CODE ON)
# Attempt to enable Link Time Optimization
#target_compile_features(${PROJECT_NAME} PUBLIC INTERPROCEDURAL_OPTIMIZATION ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        # using clang with clang-cl front end

        # Disable RTTI and exceptions
        # target_compile_options(${PROJECT_NAME} PRIVATE -fno-rtti -fno-exceptions)
        
        target_compile_options(${PROJECT_NAME} PRIVATE -Wno-unknown-attributes)
    elseif(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
        # using clang with regular front end

        # Disable RTTI and exceptions
        # target_compile_options(${PROJECT_NAME} PRIVATE -fno-rtti -fno-exceptions)
    endif()
endif()

if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    # Disable exceptions and ignore some CRT warnings
    target_compile_definitions(${PROJECT_NAME} PRIVATE -D_CRT_SECURE_NO_WARNINGS -D_HAS_EXCEPTIONS=1)

    set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE -DTAU_UTILS_IMPORT_SHARED)

check_c_compiler_flag(/wd5030 HAS_UNRECOGNIZED_ATTRIBUTES_WARNING)
check_c_compiler_flag(/wd4251 HAS_DLL_INTERFACE_WARNING)

if(HAS_UNRECOGNIZED_ATTRIBUTES_WARNING)
    target_compile_options(${PROJECT_NAME} PRIVATE /wd5030)
endif()

if(HAS_DLL_INTERFACE_WARNING)
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4251)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

install(TARGETS ${PROJECT_NAME})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
