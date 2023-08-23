cmake_minimum_required(VERSION 3.23)
project(PackageTest LANGUAGES CXX C)

include(CheckCCompilerFlag)

find_package(taucom CONFIG REQUIRED)
# add_subdirectory(../libs/TauUtils)
find_package(tauutils CONFIG REQUIRED)

add_executable(${PROJECT_NAME} src/example.cpp)
target_link_libraries(${PROJECT_NAME} taucom::taucom tauutils::TauUtilsDynamicShared)

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

target_compile_definitions(${PROJECT_NAME} PRIVATE -DTAU_UTILS_IMPORT_SHARED -DTAU_COM_BUILD_SHARED)

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

