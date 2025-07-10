cmake_minimum_required(VERSION 3.14)
project(ParsecCoreProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17) # Require C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Prefer not using GNU extensions

# --- External Dependencies ---
# Include FetchContent *after* project settings
include(FetchContent)

# Google Test
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) # Force static library for gtest
FetchContent_MakeAvailable(googletest)

# nlohmann/json (Header-only JSON library)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3 # Use a specific tag for stability
)
FetchContent_MakeAvailable(nlohmann_json)

# muParser (Expression Parser - Retrying FetchContent)
FetchContent_Declare(
  muparser
  GIT_REPOSITORY https://github.com/beltoforion/muparser.git # Correct repo
  GIT_TAG v2.3.5 # Use latest tag with CMake version fix
)
set(ENABLE_SAMPLES OFF CACHE BOOL "" FORCE) # Don't build samples
set(ENABLE_OPENMP OFF CACHE BOOL "" FORCE) # Disable OpenMP if not needed
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) # Force static library for muparser
FetchContent_MakeAvailable(muparser)
# --- End External Dependencies ---

# --- Library Code (if needed later, e.g., if ParsecCore becomes a library) ---
# add_library(parsec_core ParsecCore.cpp ConfigManager.cpp) # Add new source files if creating library
# target_include_directories(parsec_core PUBLIC
#    ${CMAKE_CURRENT_SOURCE_DIR}
#    ${nlohmann_json_SOURCE_DIR}/include # Make json headers available
# )

# Keep mathcore as INTERFACE for header-only math
add_library(mathcore INTERFACE)
target_include_directories(mathcore INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# --- Main Windows Executable (kept for manual testing/running) ---
add_executable(parsec_windows_app
    main_windows.cpp
    ParsecCore.cpp
    ConfigManager.cpp
    EquationManager.cpp
    SolverCore.cpp
    MLCore.cpp
)
target_include_directories(parsec_windows_app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/include # For mathcore and parsec headers
  ${nlohmann_json_SOURCE_DIR}/include # Include JSON headers
  ${muparser_SOURCE_DIR}/include # Include muParser headers from FetchContent
)
target_link_libraries(parsec_windows_app PRIVATE muparser) # Link against FetchContent muparser

# --- Test Executable ---
enable_testing() # Enable CTest support

# Add new source files to be compiled with tests
add_executable(parsec_tests
    tests/math_tests.cpp
    tests/config_manager_tests.cpp
    tests/solver_core_tests.cpp # Add SolverCore test file
    # Add tests/solver_core_tests.cpp later
    ConfigManager.cpp
    EquationManager.cpp # Compile EquationManager with tests
    SolverCore.cpp      # Compile SolverCore with tests
    MLCore.cpp          # Compile MLCore with tests
)

target_link_libraries(parsec_tests PRIVATE gtest_main muparser) # Link tests against FetchContent muparser

# Include directories needed by the tests
target_include_directories(parsec_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} # For IPlatform, ParsecCore.h etc.
    ${CMAKE_CURRENT_SOURCE_DIR}/include # For mathcore, parsec headers
    ${nlohmann_json_SOURCE_DIR}/include # Include JSON headers for ConfigManager tests
    ${muparser_SOURCE_DIR}/include # Include muParser headers for tests from FetchContent
)

include(GoogleTest)
gtest_discover_tests(parsec_tests)
# --- End Test Executable ---
