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

# ASIO for standalone networking
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-28-0
)
FetchContent_MakeAvailable(asio)

# websocketpp for WebSocket client
FetchContent_Declare(
    websocketpp
    GIT_REPOSITORY https://github.com/zaphoyd/websocketpp.git
    GIT_TAG 0.8.2
)
FetchContent_MakeAvailable(websocketpp)

# websocketpp and asio are header-only, so we need to add include directories
set(WEBSOCKETPP_INCLUDE_DIR ${websocketpp_SOURCE_DIR})
set(ASIO_INCLUDE_DIR ${asio_SOURCE_DIR}/asio/include)

# Source files
set(PARSEC_CORE_SOURCES
    src/parsec/InputManager.cpp
    # Add other source files here
)

# Create library
add_library(ParsecCore ${PARSEC_CORE_SOURCES})
target_include_directories(ParsecCore PUBLIC include)
target_include_directories(ParsecCore PRIVATE ${WEBSOCKETPP_INCLUDE_DIR})
target_include_directories(ParsecCore PRIVATE ${ASIO_INCLUDE_DIR})
target_link_libraries(ParsecCore PRIVATE nlohmann_json::nlohmann_json)

# Define preprocessor flags for standalone ASIO without Boost
target_compile_definitions(ParsecCore PRIVATE
    ASIO_STANDALONE
    _WEBSOCKETPP_CPP11_STL_
    _WEBSOCKETPP_CPP11_FUNCTIONAL_
    _WEBSOCKETPP_CPP11_SYSTEM_ERROR_
    _WEBSOCKETPP_CPP11_RANDOM_DEVICE_
    _WEBSOCKETPP_CPP11_MEMORY_
)

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