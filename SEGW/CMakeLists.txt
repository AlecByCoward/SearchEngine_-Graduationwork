cmake_minimum_required(VERSION 3.16)
project(SearchEngine VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Опции компиляции
option(BUILD_TESTS "Build tests" ON)
option(ENABLE_PROFILING "Enable profiling" OFF)

# Сначала пытаемся найти nlohmann_json в системе
find_package(nlohmann_json QUIET)

# Если не найдена, скачиваем через FetchContent
if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found in system, downloading...")
    include(FetchContent)
    
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
    )
    FetchContent_MakeAvailable(nlohmann_json)
else()
    message(STATUS "Using system nlohmann_json")
endif()

# Основной исполняемый файл
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/ConverterJSON.cpp
    src/InvertedIndex.cpp
    src/SearchServer.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE nlohmann_json::nlohmann_json
)

# ---- Тесты ----
if(BUILD_TESTS)
    # Сначала пытаемся найти GTest в системе
    find_package(GTest QUIET)
    
    # Если не найден, скачиваем через FetchContent
    if(NOT GTest_FOUND)
        message(STATUS "GTest not found in system, downloading...")
        include(FetchContent)
        
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.15.2
        )
        FetchContent_MakeAvailable(googletest)
    else()
        message(STATUS "Using system GTest")
    endif()

    enable_testing()

    # Собираем все тестовые cpp, если они есть
    file(GLOB TEST_SOURCES tests/*.cpp)

    if(TEST_SOURCES)
        add_executable(tests ${TEST_SOURCES})
        target_include_directories(tests PRIVATE include)
        target_link_libraries(tests
            PRIVATE
            GTest::gtest_main
            nlohmann_json::nlohmann_json
        )
        include(GoogleTest)
        gtest_discover_tests(tests)
    endif()
endif()
