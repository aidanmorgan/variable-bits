cmake_minimum_required(VERSION 3.12)
project(bit_stream C)

# Set C standard to C23
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    src/bit_value.c
    src/uint128.c
    src/bit_stream.c
    src/bit_stream_reader.c
    src/bit_stream_writer.c
)

# Create static library
add_library(bit_stream STATIC ${SOURCES})
target_include_directories(bit_stream PUBLIC src)

# Create shared library
add_library(bit_stream_shared SHARED ${SOURCES})
target_include_directories(bit_stream_shared PUBLIC src)
set_target_properties(bit_stream_shared PROPERTIES OUTPUT_NAME bit_stream)

# Install targets
install(TARGETS bit_stream bit_stream_shared
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)
install(FILES src/bit_stream.h DESTINATION include)

# Add Unity test framework
include(FetchContent)
FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG v2.5.2
)
FetchContent_MakeAvailable(unity)

# Enable testing
enable_testing()

# Add test directory
add_subdirectory(test)