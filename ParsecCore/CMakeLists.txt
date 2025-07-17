cmake_minimum_required(VERSION 3.14)
project(ParsecCore VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Find or download dependencies
include(FetchContent)

# nlohmann/json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

# Source files
set(PARSEC_CORE_SOURCES
    src/parsec/InputManager.cpp
    # Add other source files here
)

# Create library
add_library(ParsecCore ${PARSEC_CORE_SOURCES})
target_include_directories(ParsecCore PUBLIC include)
target_link_libraries(ParsecCore PRIVATE nlohmann_json::nlohmann_json)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(ParsecCore PRIVATE ws2_32)
endif()

# Examples
add_executable(input_manager_example ../examples/input_manager_example.cpp)
target_link_libraries(input_manager_example PRIVATE ParsecCore)

# Install
install(TARGETS ParsecCore
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)