cmake_minimum_required(VERSION 3.16)
project(SearchEngine VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Опции компиляции
option(BUILD_TESTS "Build tests" ON)
option(ENABLE_PROFILING "Enable profiling" OFF)

# Пакеты
find_package(nlohmann_json REQUIRED)

if(BUILD_TESTS)
    find_package(GTest REQUIRED)
    enable_testing()
endif()
