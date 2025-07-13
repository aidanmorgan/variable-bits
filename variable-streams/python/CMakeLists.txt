cmake_minimum_required(VERSION 3.12)
project(variable_streams_python)

# Find Python
find_package(Python REQUIRED COMPONENTS Interpreter Development)

# Set the Python package directory
set(PYTHON_PACKAGE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Define custom target for installing the package
add_custom_target(install_package
    COMMAND ${Python_EXECUTABLE} -m uv pip install -e ${PYTHON_PACKAGE_DIR}
    WORKING_DIRECTORY ${PYTHON_PACKAGE_DIR}
    COMMENT "Installing Python package in development mode"
)

# Define custom target for syncing dependencies
add_custom_target(sync_dependencies
    COMMAND ${Python_EXECUTABLE} -m uv sync
    WORKING_DIRECTORY ${PYTHON_PACKAGE_DIR}
    COMMENT "Syncing Python dependencies"
)

# Define custom target for running tests
add_custom_target(run_tests
    COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${PYTHON_PACKAGE_DIR}/src ${Python_EXECUTABLE} -m uv run pytest -v
    WORKING_DIRECTORY ${PYTHON_PACKAGE_DIR}
    COMMENT "Running Python tests"
)

# Define custom target for building a wheel
add_custom_target(build_wheel
    COMMAND ${Python_EXECUTABLE} -m uv pip build
    WORKING_DIRECTORY ${PYTHON_PACKAGE_DIR}
    COMMENT "Building Python wheel"
)

# Define custom target for cleaning Python artifacts
add_custom_target(clean_python
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${PYTHON_PACKAGE_DIR}/dist
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${PYTHON_PACKAGE_DIR}/build
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${PYTHON_PACKAGE_DIR}/*.egg-info
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${PYTHON_PACKAGE_DIR}/__pycache__
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${PYTHON_PACKAGE_DIR}/src/__pycache__
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${PYTHON_PACKAGE_DIR}/src/variable_streams/__pycache__
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${PYTHON_PACKAGE_DIR}/tests/__pycache__
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${PYTHON_PACKAGE_DIR}/.pytest_cache
    COMMENT "Cleaning Python artifacts"
)

# Add dependencies between targets
add_dependencies(run_tests install_package)
add_dependencies(install_package sync_dependencies)

# Default target
add_custom_target(all_python ALL DEPENDS install_package)

# Install target
install(CODE "execute_process(COMMAND ${Python_EXECUTABLE} -m uv pip install ${PYTHON_PACKAGE_DIR} WORKING_DIRECTORY ${PYTHON_PACKAGE_DIR})")