cmake_minimum_required(VERSION 3.8)

project(server_test)

# Включаем модуль FetchContent
include(FetchContent)

# Объявляем загрузку googletest
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1  # Можно указать другую версию
)

# Включаем googletest в проект
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(server_test index_tests.cpp server_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/inverted_index.cpp
        ${CMAKE_SOURCE_DIR}/src/server.cpp)

target_include_directories(server_test PUBLIC ${gtest_SOURCE_DIR}/include ${INCLUDE_DIR})

target_link_libraries(server_test PRIVATE gtest_main)
