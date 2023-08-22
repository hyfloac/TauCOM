cmake_minimum_required(VERSION 3.15)
project(PackageTest LANGUAGES CXX C)

find_package(taucom CONFIG REQUIRED)



add_executable(example src/example.cpp)
target_link_libraries(example taucom::taucom)
