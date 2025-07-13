cmake_minimum_required(VERSION 3.20)
project(variable_streams_multi_lang)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing
enable_testing()

# Find required packages
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(Java COMPONENTS Runtime REQUIRED)

# Check for Rust
find_program(CARGO_EXECUTABLE cargo)
if(NOT CARGO_EXECUTABLE)
    message(WARNING "Cargo not found. Rust tests will be skipped.")
    set(RUST_AVAILABLE FALSE)
else()
    set(RUST_AVAILABLE TRUE)
endif()

# Check for .NET
find_program(DOTNET_EXECUTABLE dotnet)
if(NOT DOTNET_EXECUTABLE)
    message(WARNING "dotnet not found. C# tests will be skipped.")
    set(DOTNET_AVAILABLE FALSE)
else()
    set(DOTNET_AVAILABLE TRUE)
endif()

# Check for Maven
find_program(MAVEN_EXECUTABLE mvn)
if(NOT MAVEN_EXECUTABLE)
    message(WARNING "Maven not found. Java tests will be skipped.")
    set(MAVEN_AVAILABLE FALSE)
else()
    set(MAVEN_AVAILABLE TRUE)
endif()

# Check for uv (Python package manager)
find_program(UV_EXECUTABLE uv)
if(NOT UV_EXECUTABLE)
    message(WARNING "uv not found. Python tests will be skipped.")
    set(UV_AVAILABLE FALSE)
else()
    set(UV_AVAILABLE TRUE)
endif()

# =============================================================================
# C Language Build and Test
# =============================================================================

message(STATUS "Configuring C language build...")

# Add C subdirectory
add_subdirectory(c23)

# Create custom target for C tests
add_custom_target(test_c
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS bit_stream_test
    COMMENT "Running C tests..."
)

# =============================================================================
# C++ Language Build and Test
# =============================================================================

message(STATUS "Configuring C++ language build...")

# Add C++ subdirectory if it exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cpp/CMakeLists.txt")
    add_subdirectory(cpp)
    
    add_custom_target(test_cpp
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS cpp_tests
        COMMENT "Running C++ tests..."
    )
else()
    message(STATUS "C++ CMakeLists.txt not found, skipping C++ build")
endif()

# =============================================================================
# Rust Language Build and Test
# =============================================================================

if(RUST_AVAILABLE)
    message(STATUS "Configuring Rust language build...")
    
    # Create custom target for Rust build and test
    add_custom_target(build_rust
        COMMAND ${CMAKE_COMMAND} -E env CARGO_TARGET_DIR=${CMAKE_BINARY_DIR}/rust/target
        ${CARGO_EXECUTABLE} build --manifest-path ${CMAKE_CURRENT_SOURCE_DIR}/rust/Cargo.toml
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/rust
        COMMENT "Building Rust project..."
    )
    
    add_custom_target(test_rust
        COMMAND ${CMAKE_COMMAND} -E env CARGO_TARGET_DIR=${CMAKE_BINARY_DIR}/rust/target
        ${CARGO_EXECUTABLE} test --manifest-path ${CMAKE_CURRENT_SOURCE_DIR}/rust/Cargo.toml
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/rust
        DEPENDS build_rust
        COMMENT "Running Rust tests..."
    )
else()
    message(STATUS "Rust not available, skipping Rust build and tests")
endif()

# =============================================================================
# Python Language Build and Test
# =============================================================================

if(UV_AVAILABLE)
    message(STATUS "Configuring Python language build...")
    
    # Sync Python dependencies using uv
    add_custom_target(install_python_dev
        COMMAND ${UV_EXECUTABLE} sync
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python
        COMMENT "Syncing Python dependencies using uv..."
    )
    
    # Run Python tests using uv
    add_custom_target(test_python
        COMMAND ${UV_EXECUTABLE} run pytest ${CMAKE_CURRENT_SOURCE_DIR}/python/tests/
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python
        DEPENDS install_python_dev
        COMMENT "Running Python tests using uv..."
    )
else()
    message(STATUS "uv not available, skipping Python tests")
endif()

# =============================================================================
# Java Language Build and Test
# =============================================================================

if(MAVEN_AVAILABLE)
    message(STATUS "Configuring Java language build...")
    
    # Build Java project
    add_custom_target(build_java
        COMMAND ${MAVEN_EXECUTABLE} clean compile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/java
        COMMENT "Building Java project..."
    )
    
    # Package Java project (create JAR)
    add_custom_target(package_java
        COMMAND ${MAVEN_EXECUTABLE} package -DskipTests
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/java
        DEPENDS build_java
        COMMENT "Packaging Java project..."
    )
    
    # Run Java tests
    add_custom_target(test_java
        COMMAND ${MAVEN_EXECUTABLE} test
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/java
        DEPENDS build_java
        COMMENT "Running Java tests..."
    )
    
    # Install Java project to local repository
    add_custom_target(install_java
        COMMAND ${MAVEN_EXECUTABLE} install -DskipTests
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/java
        DEPENDS package_java
        COMMENT "Installing Java project to local repository..."
    )
else()
    message(STATUS "Maven not available, skipping Java build and tests")
endif()

# =============================================================================
# C# Language Build and Test
# =============================================================================

if(DOTNET_AVAILABLE)
    message(STATUS "Configuring C# language build...")
    
    # Build C# project
    add_custom_target(build_dotnet
        COMMAND ${DOTNET_EXECUTABLE} build ${CMAKE_CURRENT_SOURCE_DIR}/dotnet/VariableBits.sln
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dotnet
        COMMENT "Building C# project..."
    )
    
    # Run C# tests
    add_custom_target(test_dotnet
        COMMAND ${DOTNET_EXECUTABLE} test ${CMAKE_CURRENT_SOURCE_DIR}/dotnet/VariableBits.sln
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dotnet
        DEPENDS build_dotnet
        COMMENT "Running C# tests..."
    )
else()
    message(STATUS "dotnet not available, skipping C# build and tests")
endif()

# =============================================================================
# Master Test Target
# =============================================================================

# Create a master test target that runs all available tests
add_custom_target(test_all
    COMMAND ${CMAKE_COMMAND} -E echo "Running all tests..."
    COMMENT "Running all language tests..."
)

# Add dependencies for available languages
add_dependencies(test_all test_c)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cpp/CMakeLists.txt")
    add_dependencies(test_all test_cpp)
endif()

if(RUST_AVAILABLE)
    add_dependencies(test_all test_rust)
endif()

if(UV_AVAILABLE)
    add_dependencies(test_all test_python)
endif()

if(MAVEN_AVAILABLE)
    add_dependencies(test_all test_java)
endif()

if(DOTNET_AVAILABLE)
    add_dependencies(test_all test_dotnet)
endif()

# =============================================================================
# Build All Target
# =============================================================================

# Create a master build target
add_custom_target(build_all
    COMMAND ${CMAKE_COMMAND} -E echo "Building all languages..."
    COMMENT "Building all language projects..."
)

# Add dependencies for available languages
add_dependencies(build_all bit_stream)

if(RUST_AVAILABLE)
    add_dependencies(build_all build_rust)
endif()

if(UV_AVAILABLE)
    add_dependencies(build_all install_python_dev)
endif()

if(MAVEN_AVAILABLE)
    add_dependencies(build_all package_java)
endif()

if(DOTNET_AVAILABLE)
    add_dependencies(build_all build_dotnet)
endif()

# =============================================================================
# Clean Target
# =============================================================================

# Create a clean target that cleans all languages
add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E echo "Cleaning all languages..."
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMENT "Cleaning all language projects..."
)

if(RUST_AVAILABLE)
    add_custom_target(clean_rust
        COMMAND ${CARGO_EXECUTABLE} clean --manifest-path ${CMAKE_CURRENT_SOURCE_DIR}/rust/Cargo.toml
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/rust
        COMMENT "Cleaning Rust project..."
    )
    add_dependencies(clean_all clean_rust)
endif()

if(MAVEN_AVAILABLE)
    add_custom_target(clean_java
        COMMAND ${MAVEN_EXECUTABLE} clean
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/java
        COMMENT "Cleaning Java project..."
    )
    add_dependencies(clean_all clean_java)
endif()

if(DOTNET_AVAILABLE)
    add_custom_target(clean_dotnet
        COMMAND ${DOTNET_EXECUTABLE} clean ${CMAKE_CURRENT_SOURCE_DIR}/dotnet/VariableBits.sln
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dotnet
        COMMENT "Cleaning C# project..."
    )
    add_dependencies(clean_all clean_dotnet)
endif()

# =============================================================================
# Status Report
# =============================================================================

# Print status of available languages
message(STATUS "")
message(STATUS "=== Variable Streams Multi-Language Build Status ===")
message(STATUS "C/C++: Available")
if(RUST_AVAILABLE)
    message(STATUS "Rust: Available")
else()
    message(STATUS "Rust: Not available (cargo not found)")
endif()
if(UV_AVAILABLE)
    message(STATUS "Python: Available (uv)")
else()
    message(STATUS "Python: Not available (uv not found)")
endif()
if(MAVEN_AVAILABLE)
    message(STATUS "Java: Available (Maven)")
else()
    message(STATUS "Java: Not available (mvn not found)")
endif()
if(DOTNET_AVAILABLE)
    message(STATUS "C#: Available")
else()
    message(STATUS "C#: Not available (dotnet not found)")
endif()
message(STATUS "==================================================")
message(STATUS "")
message(STATUS "Available targets:")
message(STATUS "  build_all    - Build all available languages")
message(STATUS "  test_all     - Run tests for all available languages")
message(STATUS "  clean_all    - Clean all language projects")
message(STATUS "")
message(STATUS "Individual language targets:")
message(STATUS "  test_c       - Run C tests")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cpp/CMakeLists.txt")
    message(STATUS "  test_cpp     - Run C++ tests")
endif()
if(RUST_AVAILABLE)
    message(STATUS "  test_rust    - Run Rust tests")
endif()
if(UV_AVAILABLE)
    message(STATUS "  test_python  - Run Python tests (uv)")
endif()
if(MAVEN_AVAILABLE)
    message(STATUS "  test_java    - Run Java tests")
    message(STATUS "  package_java - Package Java JAR")
    message(STATUS "  install_java - Install Java to local repo")
endif()
if(DOTNET_AVAILABLE)
    message(STATUS "  test_dotnet  - Run C# tests")
endif()
message(STATUS "") 