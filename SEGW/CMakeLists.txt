cmake_minimum_required(VERSION 3.14)
project(search_engine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(include)

# Find nlohmann_json package
include(FetchContent)

# First try to find the package in the system
find_package(nlohmann_json 3.11.2 QUIET)

if(NOT nlohmann_json_FOUND)
    # If not found, download it
    message(STATUS "nlohmann_json not found, downloading...")
    
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
    )
    
    FetchContent_GetProperties(nlohmann_json)
    if(NOT nlohmann_json_POPULATED)
        FetchContent_Populate(nlohmann_json)
        add_subdirectory(${nlohmann_json_SOURCE_DIR} ${nlohmann_json_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
    
    set(nlohmann_json_include ${nlohmann_json_SOURCE_DIR}/include)
    set(nlohmann_json_lib nlohmann_json::nlohmann_json)
else()
    set(nlohmann_json_include ${nlohmann_json_INCLUDE_DIRS})
    set(nlohmann_json_lib ${nlohmann_json_LIBRARIES})
    message(STATUS "Found nlohmann_json: ${nlohmann_json_include}")
endif()

# Add executable
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/ConverterJSON.cpp
    src/InvertedIndex.cpp
    src/SearchServer.cpp
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${nlohmann_json_include}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${nlohmann_json_lib}
)

# Add compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    -DJSON_USE_IMPLICIT_CONVERSIONS=1
)
