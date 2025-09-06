cmake_minimum_required(VERSION 3.16)

project(SearchEngineTests VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Сначала пытаемся найти библиотеки в системе
find_package(GTest QUIET)
find_package(nlohmann_json QUIET)

# Если библиотеки не найдены, скачиваем через FetchContent
if(NOT GTest_FOUND OR NOT nlohmann_json_FOUND)
    message(STATUS "Some libraries not found in system, downloading...")
    include(FetchContent)
    
    if(NOT GTest_FOUND)
        message(STATUS "Downloading GTest...")
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.15.2
        )
        FetchContent_MakeAvailable(googletest)
    else()
        message(STATUS "Using system GTest")
    endif()
    
    if(NOT nlohmann_json_FOUND)
        message(STATUS "Downloading nlohmann_json...")
        FetchContent_Declare(
            nlohmann_json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG v3.12.0
        )
        FetchContent_MakeAvailable(nlohmann_json)
    else()
        message(STATUS "Using system nlohmann_json")
    endif()
else()
    message(STATUS "Using system libraries: GTest and nlohmann_json")
endif()

enable_testing()

# Создаем исполняемый файл тестов
add_executable(SearchEngineTests 
    test_converter.cpp 
    test_inverted_index.cpp 
    test_search_server.cpp
    test_main.cpp
    ../SEGW/src/ConverterJSON.cpp
    ../SEGW/src/InvertedIndex.cpp
    ../SEGW/src/SearchServer.cpp
)

target_include_directories(SearchEngineTests 
    PUBLIC 
    ${CMAKE_SOURCE_DIR}/../SEGW/include
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(SearchEngineTests 
    PRIVATE 
    GTest::gtest_main 
    nlohmann_json::nlohmann_json
)

# Добавляем тесты в CTest
include(GoogleTest)
gtest_discover_tests(SearchEngineTests)
